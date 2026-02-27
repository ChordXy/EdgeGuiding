#include "peripheralOperator.h"



int peripheralOperator::fileOperate(const std::string attr, const std::string val) {
     optFile = optDevice + "/" + attr;
     int fd = open(optFile.c_str(), O_WRONLY);
     if (fd <= 0) {
         perror("Open ERROR");
         return fd;
     }

     int len = val.size();
     if (len != write(fd, val.c_str(), len)) {
         perror("WRITE ERROR");
         close(fd);
         return -1;
     }
     close(fd);
     return 0;
 }

int peripheralOperator::init(int port){
     expDevice = std::string(PWM_BASE) + "/export";
     optDevice = std::string(PWM_BASE) + "/pwm" + std::to_string(port);

     if (access(optDevice.c_str(), F_OK))      {
         int fd = ::open(expDevice.c_str(), O_WRONLY);
         if (fd <= 0) {
             perror("Open ERROR");
             return -1;
         }
         if (1 != write(fd, std::to_string(port).c_str(), 1)) {
             perror("WRITE ERROR");
             ::close(fd);
             exit(-1);
         }
         ::close(fd);
     }
     return 1;
 }
