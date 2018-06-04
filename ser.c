#include<stdio.h>  
#include<stdlib.h>  
#include<unistd.h>  
#include<sys/types.h>  
#include<sys/ioctl.h>  
#include<sys/stat.h>  
#include<fcntl.h>  
#include<termios.h>  
#include<errno.h>  
#include<stdlib.h>
#include<string.h>

  
#define FALSE -1  
#define TRUE 0  
  
int speed_arr[] = { B38400, B19200, B9600, B4800, B2400, B1200, B300,B38400, B19200, B9600, B4800, B2400, B1200, B300, };  
int name_arr[] = {38400,  19200,  9600,  4800,  2400,  1200,  300, 38400, 19200,  9600, 4800, 2400, 1200,  300, };  
    int next[4];

void set_speed(int fd, int speed){  
  int   i;   
  int   status;   
  struct termios   Opt;  
  tcgetattr(fd, &Opt);   
  for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++) {   
    if  (speed == name_arr[i]) {       
      tcflush(fd, TCIOFLUSH);       
      cfsetispeed(&Opt, speed_arr[i]);    
      cfsetospeed(&Opt, speed_arr[i]);     
      status = tcsetattr(fd, TCSANOW, &Opt);    
      if  (status != 0) {          
        perror("tcsetattr fd1");    
        return;       
      }      
      tcflush(fd,TCIOFLUSH);     
    }    
  }  
}  
  
int set_Parity(int fd,int databits,int stopbits,int parity)  
{   
    struct termios options;   
    if  ( tcgetattr( fd,&options)  !=  0) {   
        perror("SetupSerial 1");       
        return(FALSE);    
    }  
    options.c_cflag &= ~CSIZE;   
    switch (databits)   
    {     
    case 7:       
        options.c_cflag |= CS7;   
        break;  
    case 8:       
        options.c_cflag |= CS8;  
        break;     
    default:      
        fprintf(stderr,"Unsupported data size\n"); return (FALSE);    
    }  
    switch (parity)   
    {     
        case 'n':  
        case 'N':      
            options.c_cflag &= ~PARENB;   /* Clear parity enable */  
            options.c_iflag &= ~INPCK;     /* Enable parity checking */   
            break;    
        case 'o':     
        case 'O':       
            options.c_cflag |= (PARODD | PARENB);   
            options.c_iflag |= INPCK;             /* Disnable parity checking */   
            break;    
        case 'e':    
        case 'E':     
            options.c_cflag |= PARENB;     /* Enable parity */      
            options.c_cflag &= ~PARODD;      
            options.c_iflag |= INPCK;       /* Disnable parity checking */  
            break;  
        case 'S':   
        case 's':  /*as no parity*/     
            options.c_cflag &= ~PARENB;  
            options.c_cflag &= ~CSTOPB;break;    
        default:     
            fprintf(stderr,"Unsupported parity\n");      
            return (FALSE);    
        }    
      
    switch (stopbits)  
    {     
        case 1:      
            options.c_cflag &= ~CSTOPB;    
            break;    
        case 2:      
            options.c_cflag |= CSTOPB;    
           break;  
        default:      
             fprintf(stderr,"Unsupported stop bits\n");    
             return (FALSE);   
    }   
    /* Set input parity option */   
    if (parity != 'n')     
        options.c_iflag |= INPCK;   
    tcflush(fd,TCIFLUSH);  
    options.c_cc[VTIME] = 150;   
    options.c_cc[VMIN] = 0; /* Update the options and do it NOW */  
    if (tcsetattr(fd,TCSANOW,&options) != 0)     
    {   
        perror("SetupSerial 3");     
        return (FALSE);    
    }   
    return (TRUE);    
} 

int main()  
{  
    printf("This program updates last time at %s   %s\n",__TIME__,__DATE__);  
    printf("STDIO COM1\n");  
    int fd;  
    fd = open("/dev/ttyS0",O_RDWR);  
    if(fd == -1)  
    {  
        perror("serialport error\n");  
    }  
    else  
    {  
        printf("open ");  
        printf("%s",ttyname(fd));  
        printf(" succesfully\n");  
    }  
  
    set_speed(fd,115200);  
    if (set_Parity(fd,8,1,'N') == FALSE)  {  
        printf("Set Parity Error\n");  
        exit (0);  
    }  
    char buff[512]="";
    int nread, ct=0;
    int tmp;
    int buf_cnt;
    int buf_idx;
    int stat;

    char des[5] = "22222";//标识码

    fd_set read_fds;
    // int ser_fd;//ser_fd ??
    struct timeval timeout;


    while(1){
        FD_ZERO(&read_fds);
        FD_SET(fd, &read_fds);

        timeout.tv_sec= 6;
        timeout.tv_usec = 0;

        stat = select(FD_SETSIZE, &read_fds, NULL, NULL, &timeout);
        if (stat > 0) { // there is sth to read
            if (FD_ISSET(fd, &read_fds)) {
                tmp = read(fd, buff, 512);
                buf_cnt = buf_cnt + tmp;//?? 似乎不需要
                // buf_idx = buf_idx + tmp;//??  似乎不需要
                printf("tmp is: %d\n",tmp);
            }
        }else if(stat < 0){ // error
            perror("select error\n");
        }else{ // 超时则视为消息发送结束, 此时处理消息
            printf("timeout\n");
            printf("the whole buff is: %s\n", buff);

            //字符串处理

            // 强制清空buff
            // printf("now reset the buff");
            // memset(buff, 0, sizeof(buff));
        }
    }
    // while(1)  
    // {  
    //     if((nread = read(fd, buff, 512))>0)  
    //     {  
    //         buff[nread+1] = '\0';
    //         char c = buff[0];
    //         if (ct >= 5){
    //             printf("%c\n",c);  
    //         }else{
    //             if(c == '2'){
    //                 ct++;
    //             }else{
    //                 ct = 0;
    //             }
    //         }
    //     }  
    // }  
    close(fd);  
    return 0;  
}  