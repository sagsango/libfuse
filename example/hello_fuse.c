/*
This code was written with the help of chatgpt for demonstration purpose.

1. We dont need to build the libfuse source;
sudo apt-get update
sudo apt-get install libfuse-dev gcc

2. Compile
gcc -o hello_fuse hello_fuse.c -lfuse

3. Run
mkdir mountpoint
./hello_fuse mountpoint

3. Test
ls mountpoint
cat mountpoint/hello.txt

4. unmount
umount /home/ssing214/fuse-lab/mountpoint
*/


#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>

// The content of the file hello.txt
static const char *hello_str = "Hello, World!\n";
static const char *hello_path = "/hello.txt";

// Log file pointer
FILE *log_file = NULL;

// Function to log debug messages
void log_debug(const char *message) {
    if (log_file == NULL) {
        log_file = fopen("fuse_debug.log", "a");
        if (log_file == NULL) {
            perror("Failed to open log file");
            exit(EXIT_FAILURE);
        }
    }

    fprintf(log_file, "[DEBUG] %s\n", message);
    fflush(log_file);
}

// Function to get the attributes of a file (like stat)
static int hello_getattr(const char *path, struct stat *stbuf) {
    log_debug("Called hello_getattr");
    
    memset(stbuf, 0, sizeof(struct stat));
    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
    } else if (strcmp(path, hello_path) == 0) {
        stbuf->st_mode = S_IFREG | 0444;
        stbuf->st_nlink = 1;
        stbuf->st_size = strlen(hello_str);
    } else {
        return -ENOENT;
    }
    return 0;
}

// Function to open a file
static int hello_open(const char *path, struct fuse_file_info *fi) {
    log_debug("Called hello_open");

    if (strcmp(path, hello_path) != 0) {
        return -ENOENT;
    }

    if ((fi->flags & O_ACCMODE) != O_RDONLY) {
        return -EACCES;
    }

    return 0;
}

// Function to read the content of the file
static int hello_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    log_debug("Called hello_read");

    size_t len;
    (void) fi;
    if(strcmp(path, hello_path) != 0) {
        return -ENOENT;
    }

    len = strlen(hello_str);
    if (offset < len) {
        if (offset + size > len) {
            size = len - offset;
        }
        memcpy(buf, hello_str + offset, size);
    } else {
        size = 0;
    }

    return size;
}

// Fuse operations structure
static struct fuse_operations hello_oper = {
    .getattr = hello_getattr,
    .open    = hello_open,
    .read    = hello_read,
};

// Main function to mount the filesystem
int main(int argc, char *argv[]) {
    log_debug("Starting FUSE filesystem");
    int ret = fuse_main(argc, argv, &hello_oper, NULL); /* XXX: All the magic happens here */

    if (log_file) {
        fclose(log_file);
    }

    return ret;
}

