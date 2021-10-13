#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/process.h"
#include "devices/shutdown.h"
#include "devices/input.h"
#include "pagedir.h"
#include "filesys/off_t.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "threads/synch.h"
#include <string.h>

#define MAX_FILE_NUM 128

struct lock filesys_lock;

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  lock_init(&filesys_lock);
}

void check_validity_of_address(void *address)
{
	struct thread *t = thread_current();
	if(is_user_vaddr(address) && !is_kernel_vaddr(address) && pagedir_get_page(t->pagedir, address))
		;		// do nothing
	else
		exit(-1);
}

void syscall_argument(int count, void* args[], void* esp){
	for(int i=0; i<count; i++){
		// (*esp +) void pointer : 4 byte
		args[i] = (void*)((unsigned*)esp+i+1);
		check_validity_of_address(args[i]);
	}
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  //printf ("system call!\n");
  void *type = (f->esp);
  void *args[4];

  switch(*(int*)type){
  	case SYS_HALT:
		halt();
		break;
	case SYS_EXIT:
		syscall_argument(1, args, f->esp);
		exit((int)*(int*)args[0]);
		break;
	case SYS_EXEC:
		syscall_argument(1, args, f->esp);
		f->eax = exec((char*)*(unsigned*)args[0]);
		break;
	case SYS_WAIT:
		syscall_argument(1, args, f->esp);
		f->eax = wait((int)*(int*)args[0]);
		break;
	case SYS_WRITE:
		syscall_argument(3, args, f->esp);
		f->eax = write((int)*(int*)args[0], (void*)*(int*)args[1], (size_t)*(int*)args[2]);
		break;
	case SYS_READ:
		syscall_argument(3, args, f->esp);
		f->eax = read((int)*(int*)args[0], (void*)*(int*)args[1], (size_t)*(int*)args[2]);
  		break;
	case SYS_FIBO:
		syscall_argument(1, args, f->esp);
		f->eax = fibonacci((int)*(int*)args[0]);
		break;
	case SYS_MAX:
		syscall_argument(4, args, f->esp);
		f->eax = max_of_four_int((int)*(int*)args[0], (int)*(int*)args[1], (int)*(int*)args[2], (int)*(int*)args[3]);
		break;
	case SYS_CREATE:
		syscall_argument(2, args, f->esp);
		f->eax = create((const char*)*(unsigned*)args[0], (unsigned)*(unsigned*)args[1]);
 		break;
	case SYS_REMOVE:
		syscall_argument(1, args, f->esp);
		f->eax = remove((const char*)*(unsigned*)args[0]);
		break;
	case SYS_OPEN:
		syscall_argument(1, args, f->esp);
		f->eax = open((const char*)*(unsigned*)args[0]);
		break;
	case SYS_CLOSE:
		syscall_argument(1, args, f->esp);
		close((int)*(int*)args[0]);
		break;
	case SYS_FILESIZE:
		syscall_argument(1, args, f->esp);
		f->eax = filesize((int)*(int*)args[0]);
		break;
	case SYS_SEEK:
		syscall_argument(2, args, f->esp);
		seek((int)*(int*)args[0], (unsigned)*(unsigned*)args[1]);
		break;
	case SYS_TELL:
		syscall_argument(1, args, f->esp);
		f->eax = tell((int)*(int*)args[0]);
		break;
	default:
		break;
  }

	//thread_exit ();
}

void halt()
{
	shutdown_power_off();
}

void exit(int status)
{
	struct thread* cur = thread_current();
	cur->exit_status = status;

	printf("%s: exit(%d)\n", thread_name(), status);
	
	for(int i=3; i<MAX_FILE_NUM; i++){
		if(cur->fd[i] != NULL)
			close(i);
	}

	thread_exit();
}

pid_t exec(const char* cmd_line)
{
	return process_execute(cmd_line);
}

int wait(pid_t pid)
{
	return process_wait(pid);
}

int write(int fd, const void* buffer, unsigned size)
{
	int res;
	struct thread *cur = thread_current();

	check_validity_of_address(buffer);
	lock_acquire(&filesys_lock);

	if(fd == 1){
		putbuf((char*)buffer, size);
		lock_release(&filesys_lock);
		return size;
	}
	else if(fd > 2){
		if(cur->fd[fd] == NULL){
			lock_release(&filesys_lock);
			exit(-1);
		}
		res = file_write(cur->fd[fd], buffer, size);
		lock_release(&filesys_lock);
		return res;
	}
	lock_release(&filesys_lock);
	return -1;
}

int read(int fd, void* buffer, unsigned size)
{
	int res;
	struct thread *cur = thread_current();
	
	check_validity_of_address(buffer);
	lock_acquire(&filesys_lock);
	
	if(fd == 0){
		for(unsigned int i=0; i<size; i++)
			((char*)buffer)[i]=(char)input_getc();
		lock_release(&filesys_lock);
		return size;
	}
	else if(fd > 2){
		if(cur->fd[fd] == NULL){
			lock_release(&filesys_lock);
			exit(-1);
		}
		res = file_read(cur->fd[fd], buffer, size);
		lock_release(&filesys_lock);
		return res;
	}
	
	lock_release(&filesys_lock);
	return -1;
}

int fibonacci(int n)
{
	int f1=1, f2=1, f3;
	if(n<0)
		return 0;
	else if(n<3)
		return 1;
	else{
		for(int i=3; i<=n; i++){
			f3=f1+f2;
			f1=f2;
			f2=f3;
		}
		return f3;
	}
}

int max_of_four_int(int a, int b, int c, int d)
{
	int max=a;
	if(max<b)
		max=b;
	if(max<c)
		max=c;
	if(max<d)
		max=d;
	return max;
}

bool create(const char *file, unsigned initial_size)
{
	if(file==NULL)
		exit(-1);
	return filesys_create(file, initial_size);
}

bool remove(const char* file)
{
	if(file==NULL)
		exit(-1);
	return filesys_remove(file);
}

int open(const char* file){
	int fd;
	struct file* fp;
	struct thread *cur = thread_current();
	int ret = -1;

	if(file==NULL)
		exit(-1);

	lock_acquire(&filesys_lock);
	fp = filesys_open(file);

	if(fp==NULL){
		ret = -1;
	}
	else{
		for(int i=3; i<MAX_FILE_NUM; i++){
			if(cur->fd[i]==NULL){
				if(!strcmp(cur->name, file))
					file_deny_write(fp);
				cur->fd[i] = fp;
				ret = i;
				break;
			}
		}
	}

	lock_release(&filesys_lock);
	return ret;
}

void close(int fd)
{
	if(fd < 3)
		return;
	struct thread *cur = thread_current();

	if(cur->fd[fd] == NULL)
		exit(-1);

	//lock_acquire(&filesys_lock);
	file_close(cur->fd[fd]);
	cur->fd[fd] = NULL;
	//lock_release(&filesys_lock);
}

int filesize(int fd)
{
	if(fd < 3)
		return -1;

	struct thread *cur = thread_current();

	if(cur->fd[fd] == NULL)
		exit(-1);

	return file_length(cur->fd[fd]);
}

void seek(int fd, unsigned position)
{
	if(fd < 3)
		return;

	struct thread *cur = thread_current();

	if(cur->fd[fd] == NULL)
		exit(-1);

	file_seek(cur->fd[fd], position);
}

unsigned tell(int fd)
{
	if(fd < 3)
		return -1;

	struct thread *cur = thread_current();

	if(cur->fd[fd] == NULL)
		exit(-1);

	return file_tell(cur->fd[fd]);
}
