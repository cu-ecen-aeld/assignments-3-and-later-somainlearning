#include "systemcalls.h"
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <syslog.h>

/**
 * @param cmd the command to execute with system()
 * @return true if the command in @param cmd was executed
 *   successfully using the system() call, false if an error occurred,
 *   either in invocation of the system() call, or if a non-zero return
 *   value was returned by the command issued in @param cmd.
*/
bool do_system(const char *cmd)
{
    openlog("do_system_fn_logs", LOG_PID | LOG_CONS, LOG_USER);
    syslog(LOG_INFO, "Entry with cmd: %s", cmd);
    int status = system(cmd);
    bool rv = false;
    if (status == -1) 
    {
	perror("system");
	rv = false;
    } 
    else if (WIFEXITED(status)) 
    {
    	syslog(LOG_INFO, "Success with code: %d", WEXITSTATUS(status));
	rv = true;
    } 
    else if (WIFSIGNALED(status)) 
    {
    	syslog(LOG_ERR, "Killed by signal: %d", WTERMSIG(status));
	rv = false;
    } 
    else if (WIFSTOPPED(status)) 
    {
    	syslog(LOG_ERR, "Stopped by signal: %d", WSTOPSIG(status));
	rv = false;
    } 
    else 
    {
    	syslog(LOG_ERR, "Unknown error");
	rv = false;
    }
    
    syslog(LOG_INFO, "Exited with rv: %d", rv);
    closelog();
    return rv;
}

/**
* @param count -The numbers of variables passed to the function. The variables are command to execute.
*   followed by arguments to pass to the command
*   Since exec() does not perform path expansion, the command to execute needs
*   to be an absolute path.
* @param ... - A list of 1 or more arguments after the @param count argument.
*   The first is always the full path to the command to execute with execv()
*   The remaining arguments are a list of arguments to pass to the command in execv()
* @return true if the command @param ... with arguments @param arguments were executed successfully
*   using the execv() call, false if an error occurred, either in invocation of the
*   fork, waitpid, or execv() command, or if a non-zero return value was returned
*   by the command issued in @param arguments with the specified arguments.
*/

bool do_exec(int count, ...)
{
    bool rv = false;
    int status = 0;
    pid_t pid = 0;

    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;

    va_end(args);

    pid = fork();

    if (pid < 0) 
    {
        perror("fork");
        rv = false;
    } 
    else if (pid == 0) 
    {
        execv(command[0], command);
        perror("execv");
        _exit(127);
    } 
    else 
    {
	if (waitpid(pid, &status, 0) == -1) 
	{
	    perror("waitpid");
	    rv = false;
	}
	if (WIFEXITED(status)) 
	{ 
	    rv = (0 == WEXITSTATUS(status)); 
	} 
	else if (WIFSIGNALED(status)) 
	{ 
	    rv = false; 
	} 
	else 
	{ 
	    rv = false; 
	}
    }

    return rv;
}

/**
* @param outputfile - The full path to the file to write with command output.
*   This file will be closed at completion of the function call.
* All other parameters, see do_exec above
*/
bool do_exec_redirect(const char *outputfile, int count, ...)
{
    bool rv = false;
    int status = 0;
    pid_t pid = 0;

    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    va_end(args);

    pid = fork();
    if (pid < 0) 
    {
        perror("fork");
        rv = false;
    } 
    else if (pid == 0) 
    {    
	int fd = open(outputfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) 
	{
            perror("open");
            _exit(127);
        }

        if (dup2(fd, STDOUT_FILENO) < 0) 
	{
            perror("dup2");
            close(fd);
            _exit(127);
        }
        close(fd);

        execv(command[0], command);
        perror("execv");
        _exit(127);
    } 
    else 
    {
	if (waitpid(pid, &status, 0) == -1) 
	{
	    perror("waitpid");
	    rv = false;
	}
	if (WIFEXITED(status)) 
	{ 
	    rv = (0 == WEXITSTATUS(status)); 
	} 
	else if (WIFSIGNALED(status)) 
	{ 
	    rv = false; 
	} 
	else 
	{ 
	    rv = false; 
	}
    }

    return rv;
}
