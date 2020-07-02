#ifndef LOG_H
#define LOG_H

// TODO: Proper log function
#define LOG(...) log_melicus(__VA_ARGS__)

void log_melicus(const char *format, ...);

#endif // LOG_H
