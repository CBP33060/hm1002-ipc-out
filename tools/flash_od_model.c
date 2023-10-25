/*
 */
#define PROGRAM_NAME "flash_od_model"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <mtd/mtd-user.h>
#include <getopt.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>

typedef int bool;
#define true 1
#define false 0

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

/* for debugging purposes only */
#ifdef DEBUG
#undef DEBUG
#define DEBUG(fmt,args...) { log_printf (LOG_ERROR,"%d: ",__LINE__); log_printf (LOG_ERROR,fmt,## args); }
#else
#undef DEBUG
#define DEBUG(fmt,args...)
#endif

#define KB(x) ((x) / 1024)
#define PERCENTAGE(x,total) (((x) * 100) / (total))

/* size of read/write buffer */
#define BUFSIZE (10 * 1024)

/* cmd-line flags */
#define FLAG_NONE		0x00
#define FLAG_VERBOSE	0x01
#define FLAG_HELP		0x02
#define FLAG_FILENAME	0x04
#define FLAG_DEVICE		0x08

/* error levels */
#define LOG_NORMAL	1
#define LOG_ERROR	2

#define SHARED_PATTH_NAME "/etc/passwd"
#define SHARED_PROJ_ID (12345)
#define RINGBUF_SIZE (6 * 1024 * 1024)


static void log_printf (int level,const char *fmt, ...)
{
	FILE *fp = level == LOG_NORMAL ? stdout : stderr;
	va_list ap;
	va_start (ap,fmt);
	vfprintf (fp,fmt,ap);
	va_end (ap);
	fflush (fp);
}

static void showusage(bool error)
{
	int level = error ? LOG_ERROR : LOG_NORMAL;

	log_printf (level,
			"\n"
			// "Flash Copy - Written by Abraham van der Merwe <abraham@2d3d.co.za>\n"
			// "\n"
			"usage: %1$s [ -v | --verbose ] <filename> <device>\n"
			"       %1$s -h | --help\n"
			"\n"
			"   -h | --help      Show this help message\n"
			"   -v | --verbose   Show progress reports\n"
			// "   <filename>       File which you want to copy to flash\n"
			"   <device>         Flash device to read from (e.g. /dev/mtd0, /dev/mtd1, etc.)\n"
			"\n",
			PROGRAM_NAME);

	exit (error ? EXIT_FAILURE : EXIT_SUCCESS);
}

static int safe_open (const char *pathname,int flags)
{
	int fd;

	fd = open (pathname,flags);
	if (fd < 0)
	{
		log_printf (LOG_ERROR,"While trying to open %s",pathname);
		if (flags & O_RDWR)
			log_printf (LOG_ERROR," for read/write access");
		else if (flags & O_RDONLY)
			log_printf (LOG_ERROR," for read access");
		else if (flags & O_WRONLY)
			log_printf (LOG_ERROR," for write access");
		log_printf (LOG_ERROR,": %m\n");
		exit (EXIT_FAILURE);
	}

	return (fd);
}

static void safe_read (int fd,const char *filename,void *buf,size_t count,bool verbose)
{
	ssize_t result;

	result = read (fd,buf,count);
	if (count != result)
	{
		if (verbose) log_printf (LOG_NORMAL,"\n");
		if (result < 0)
		{
			log_printf (LOG_ERROR,"While reading data from %s: %m\n",filename);
			exit (EXIT_FAILURE);
		}
		log_printf (LOG_ERROR,"Short read count returned while reading from %s\n",filename);
		exit (EXIT_FAILURE);
	}
}

static void safe_rewind (int fd,const char *filename)
{
	if (lseek (fd,0L,SEEK_SET) < 0)
	{
		log_printf (LOG_ERROR,"While seeking to start of %s: %m\n",filename);
		exit (EXIT_FAILURE);
	}
}

/******************************************************************************/

static int dev_fd = -1;

static void cleanup (void)
{
	if (dev_fd > 0) close (dev_fd);
}

int main (int argc,char *argv[])
{
	const char *device = NULL;
	int i,flags = FLAG_NONE;
	ssize_t result;
	size_t size,written;
	int shm_id;
    key_t shm_key;
	void *shared_memory = NULL;
	struct mtd_info_user mtd;
	unsigned char dest[BUFSIZE];

	if (argc != 2)
    {
        printf("e.g. %s /dev/mtd0\n", argv[0]);
        exit(0);
    }

	atexit (cleanup);

	device = argv[1];

	/* get some info about the flash device */
	dev_fd = safe_open (device,O_SYNC | O_RDWR);
	if (ioctl (dev_fd,MEMGETINFO,&mtd) < 0)
	{
		DEBUG("ioctl(): %m\n");
		log_printf (LOG_ERROR,"This doesn't seem to be a valid MTD flash device!\n");
		exit (EXIT_FAILURE);
	}
	log_printf (LOG_NORMAL,"mtd.size: %lu ", mtd.size);
	
    if ((shm_key = ftok(SHARED_PATTH_NAME, SHARED_PROJ_ID)) == -1) {
		DEBUG("ftok(): %m\n");
		log_printf (LOG_ERROR,"ftok shm error!\n");
        exit(EXIT_FAILURE);
    }

    shm_id = shmget(shm_key, RINGBUF_SIZE, IPC_CREAT | 0600);//IPC_EXCL
    if (shm_id < 0) {
		DEBUG("shmget(): %m\n");
		log_printf (LOG_ERROR,"shmget error!\n");
        exit(EXIT_FAILURE);
    }

    shared_memory = shmat(shm_id, NULL, 0);
    if (shared_memory == (void *)-1) {
       	DEBUG("shmat(): %m\n");
		log_printf (LOG_ERROR,"shmat error!\n");
        exit(EXIT_FAILURE);
    }
	log_printf (LOG_NORMAL, "Memory attached at %p\n", shared_memory );

	size = RINGBUF_SIZE;
	result = 0;
	while (size)
	{
		result = read (dev_fd, shared_memory + result, size);
		if (result < 0)
		{
			log_printf (LOG_ERROR,"While reading data from %s: %m\n",device);
			exit (EXIT_FAILURE);
		}
		printf("reading data result=[%d]\n",result);
		size -= result;
	}

	shmdt( shared_memory );
	log_printf (LOG_NORMAL,
				"\rread data: %luk/%luk (100%%)\n",
				KB (RINGBUF_SIZE),
				KB (RINGBUF_SIZE));

	system("touch /tmp/od_model");

	exit (EXIT_SUCCESS);
}

