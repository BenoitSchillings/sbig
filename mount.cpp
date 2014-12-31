#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>


char *port = "/dev/tty.usbserial";



int init_serial_input () {
    int fd = 0;
    struct termios options;
    
    fd = open(port, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1)
        return fd;
    fcntl(fd, F_SETFL, 0);
    tcgetattr(fd, &options);
    
    cfmakeraw(&options);
    options.c_cc[VMIN] = 1;
    options.c_cc[VTIME] = 10;

    cfsetospeed(&options, B9600);
    cfsetispeed(&options, B9600); 
    options.c_cflag |= (CLOCAL | CREAD | CS8);
    options.c_cflag &= ~(PARENB | PARODD); 
    options.c_iflag &= ~(IXON | IXOFF | IXANY | CCTS_OFLOW | CRTS_IFLOW); 
    
    options.c_cflag &= ~CSTOPB;
    tcsetattr(fd, TCSANOW, &options);
    return fd;
}

void write_serial(int fd, char *str) {
    write(fd, str, strlen(str));
    printf("\n");

    tcflush(fd, TCIOFLUSH);
}

void read_serial(int fd) {
    char c = 0;
    
    read(fd, &c, 1);
    printf("got %c\n", c); 
    while (c != '\n') {
        read(fd, &c, 1);
        printf("read %c\n", c);
    }
    printf("\n");
}

int main()
{
    int pp = init_serial_input();
    
    printf("%d\n", pp);
    write_serial(pp, "#\n"); 
    write_serial(pp, ":GC#\n");
    read_serial(pp);
    return 0;
}
