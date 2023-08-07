#include "my_fuse_loop.h"

// Global variable to indicate if the loop should stop
volatile sig_atomic_t stop = 0;
struct fuse_session *my_se;
FILE *my_log_file;
std::unordered_map<uint64_t, std::chrono::high_resolution_clock::time_point> writeStartTime;
std::unordered_map<uint64_t, std::chrono::high_resolution_clock::time_point> readStartTime;


std::unordered_map<uint64_t,std::chrono::nanoseconds> write_time;
std::unordered_map<uint64_t,std::chrono::nanoseconds> read_time;



std::unordered_map<pthread_t,uint64_t> write_ops;
std::unordered_map<pthread_t,uint64_t> read_ops;

int my_fuse_session_receive_buf_int(struct fuse_session *se, struct fuse_buf *buf,
                                    struct fuse_chan *ch,struct io_uring *ring)
{
    for(int i=0;i<BATCH_SIZE;i++){
        if (!buf[i].mem) {
            buf[i].mem = malloc(se->bufsize);
            if (!buf[i].mem) {
                printf("fuse: failed to allocate read buffer\n");
                exit(1);
            }
        }
    }

    for(int i=0;i<BATCH_SIZE;i++){
        // Prepare the read request using liburing
        struct io_uring_sqe *sqe = io_uring_get_sqe(ring);
        io_uring_prep_read(sqe,  se->fd, buf[i].mem, se->bufsize, 0);
        sqe->flags |= IOSQE_ASYNC;
        sqe->flags |= IOSQE_IO_LINK;

        io_uring_sqe_set_data(sqe, &buf[i]);
    }
    io_uring_submit(ring);
    return 0;
}


int my_fuse_session_receive_buf_int(struct fuse_session *se, struct fuse_buf *buf)
{
    ssize_t res;
    if (!buf->mem) {
        buf->mem = malloc(se->bufsize);
    }
    res = read(se->fd, buf->mem, se->bufsize);
    buf->size = res;
    return res;
}

// Function to write request data to the log file with its size as a prefix
void write_request_to_log(FILE *log_file, const void *request, size_t request_len) {
    // Write the size of the request as a 4-byte binary value
    fwrite(&request_len, sizeof(size_t), 1, log_file);
    // Write the actual request data
    fwrite(request, sizeof(char), request_len, log_file);
    fflush(log_file); // Flush the stream to ensure data is written to the file
}


// Function to read the next request from the log file
int read_request_from_log(FILE *log_file, void **request, size_t *request_len) {
    // Read the size of the request from the log file
    size_t size_read;
    if (fread(&size_read, sizeof(size_t), 1, log_file) != 1) {
        return 0; // Return 0 on EOF or error
    }

    // Allocate memory for the request and read the data
    *request = (char *)malloc(size_read);
    if (fread(*request, sizeof(char), size_read, log_file) != size_read) {
        free(*request);
        *request = NULL;
        return 0; // Return 0 on incomplete read or error
    }
    *request_len = size_read;
    return 1; // Return 1 on successful read
}

// Signal handler function to catch SIGINT (Ctrl+C)
void sigint_handler(int signum) {

    fclose(my_log_file);
    FILE *log_file = fopen("/var/log/log_file.txt", "r");
    std::cout << "Received: " << signum << std::endl;

    size_t request_len;
    struct fuse_buf fbuf{
            .mem = NULL
    };
    if (!fbuf.mem) {
        fbuf.mem = malloc(my_se->bufsize);
    }

    // Discard init
    auto start = std::chrono::high_resolution_clock::now();
    read_request_from_log(log_file, &fbuf.mem, &request_len);
    while (read_request_from_log(log_file, &fbuf.mem, &request_len)) {
        // Process the request

        fuse_session_process_buf(my_se,&fbuf);
        //process_request(request, request_len);
        free(fbuf.mem); // Free the allocated memory for the request
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    double elapsed_time = elapsed_seconds.count();
    uint64_t totalIOPS = 0;
    // Iterate through the unordered_map using range-based for loop (C++11 and later)
    for (const auto& pair : write_ops) {
        // 'pair.first' will be the key (pthread_t) and 'pair.second' will be the value (uint64_t)
        totalIOPS += pair.second;
    }

    // Compute the average write latency
    std::chrono::nanoseconds total_write_latency = std::chrono::nanoseconds(0);
    for (auto it = write_time.begin(); it != write_time.end(); ++it) {
        total_write_latency += it->second;
    }
    double average_write_latency = static_cast<double>(total_write_latency.count()) / write_time.size();

    // Compute the average read latency
    std::chrono::nanoseconds total_read_latency = std::chrono::nanoseconds(0);
    for (auto it = read_time.begin(); it != read_time.end(); ++it) {
        total_read_latency += it->second;
    }
    double average_read_latency = static_cast<double>(total_read_latency.count()) / read_time.size();


    auto IOPS = totalIOPS / elapsed_time ;

    printf("aaa");

}

int my_fuse_session_loop(struct fuse_session *se) {
    my_se = se;
    int res = 0;
    struct fuse_buf fbuf = {
            .mem = NULL,
    };

    FILE *log_file = fopen("/var/log/log_file.txt", "w");
    if (!log_file) {
        perror("Failed to open log file for writing");
        return -1; // Return an error if log file cannot be opened
    }
    my_log_file = log_file;
    pid_t pid = getpid();
    printf("Process ID (PID): %d\n", pid);
    // Set up the signal handler for SIGINT (Ctrl+C)
    signal(SIGALRM, sigint_handler);

    while (!fuse_session_exited(se) && !stop) {
        auto start = std::chrono::high_resolution_clock::now();
        res = my_fuse_session_receive_buf_int(se, &fbuf);
        // Log the received request to the log file
        write_request_to_log(log_file, fbuf.mem, res);


        if (res == -EINTR)
            continue;
        if (res <= 0)
            break;
        struct my_fuse_in_header *in = static_cast<struct my_fuse_in_header *>(fbuf.mem);
        //std::cout << "Opcode is: " << in->opcode << std::endl;
        if(in->opcode==16)
            writeStartTime[in->unique] = start;

        if(in->opcode==15)
            readStartTime[in->unique] = start;


        fuse_session_process_buf(se, &fbuf);
    }
}

int my_fuse_session_loop_uring(struct fuse_session *se) {
    struct io_uring ring;
    int ret = io_uring_queue_init(QUEUE_DEPTH, &ring, 0);
    if (ret < 0) {
        //fprintf(stderr, "io_uring_queue_init error: %s\n", strerror(-ret));
        exit(1);
    }

    int res = 0;
    struct fuse_buf fbuf[BATCH_SIZE]{};
    for (auto &i: fbuf)
        i.mem = NULL;


    while (!fuse_session_exited(se)) {
        // Use liburing to read data into fbuf
        my_fuse_session_receive_buf_int(se, fbuf, NULL, &ring);

        for (int i = 0; i < BATCH_SIZE; i++) {
            // Wait for completion event
            struct io_uring_cqe *cqe;
            int ret = io_uring_wait_cqe(&ring, &cqe);

            if (ret < 0) {
                if (ret == -EINTR || ret == -EINVAL) {
                    printf("No pending requests");
                }
                //fprintf(stderr, "io_uring_wait_cqe error: %s\n", strerror(-ret));
                exit(1);
            }

            // Process the completion event
            if (cqe->res >= 0) {
                fbuf[0].size = cqe->res;
                fuse_session_process_buf(se, &fbuf[0]);
            } //else {
            //fprintf(stderr, "Read error: %s\n", strerror(-cqe->res));
            //}

            // Mark the completion event as seen
            io_uring_cqe_seen(&ring, cqe);
        }
    }
}
