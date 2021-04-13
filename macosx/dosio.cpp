#include <compiler.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <dosio.h>
#include <unistd.h>
#include <dirent.h>
#include <unistd.h>

#if defined(_WINDOWS)
static	OEMCHAR	curpath[MAX_PATH] = OEMTEXT(".\\");
#else
static	OEMCHAR	curpath[MAX_PATH] = OEMTEXT("./");
#endif
static	OEMCHAR	*curfilep = curpath + 2;

#define ISKANJI(c)	(((((c) ^ 0x20) - 0xa1) & 0xff) < 0x3c)

void dosio_init(void) { }
void dosio_term(void) { }

											// ファイル操作
FILEH file_open(const char *path) {
	FILEH hRes = NULL;

	hRes = fopen(path, "rb+");

	return hRes;
}

FILEH file_open_rb(const char *path) {
	FILEH hRes = NULL;

	hRes = fopen(path, "rb");

	return hRes;
}

FILEH file_create(const char *path) {
    FILEH hRes = NULL;

    hRes = fopen(path, "wb+");

    return hRes;
}

long file_seek(FILEH handle, long pointer, int method) {
	fseek(handle, pointer, method);
	return(ftell(handle));
}

UINT file_read(FILEH handle, void *data, UINT length) {
	return((UINT)fread(data, 1, length, handle));
}

UINT file_write(FILEH handle, const void *data, UINT length) {
	return((UINT)fwrite(data, 1, length, handle));
}

short file_close(FILEH handle) {
	fclose(handle);
	return(0);
}

UINT file_getsize(FILEH handle) {
	struct stat sb;

	if (fstat(fileno(handle), &sb) == 0)
	{
		return (FILELEN)sb.st_size;
	}
	return(0);
}

static BRESULT cnv_sttime(time_t *t, DOSDATE *dosdate, DOSTIME *dostime) {
	struct tm	*ftime;

	ftime = localtime(t);
	if (ftime == NULL) {
		return(FAILURE);
	}
	if (dosdate) {
		dosdate->year = ftime->tm_year + 1900;
		dosdate->month = ftime->tm_mon + 1;
		dosdate->day = ftime->tm_mday;
	}
	if (dostime) {
		dostime->hour = ftime->tm_hour;
		dostime->minute = ftime->tm_min;
		dostime->second = ftime->tm_sec;
	}
	return(SUCCESS);
}

short file_getdatetime(FILEH handle, DOSDATE *dosdate, DOSTIME *dostime) {
	struct stat sb;

	if (fstat(fileno(handle), &sb) == 0) {
		if (cnv_sttime(&sb.st_mtime, dosdate, dostime) == SUCCESS) {
			return(0);
		}
	}
	return(-1);
}

short file_delete(const char *path) {
	return(remove(path));
}

short file_attr(const char *path) {
	short	attr = 0;
	
	struct stat sb;
	if(stat(path, &sb) == 0) {
		if(S_ISDIR(sb.st_mode)) {
			attr |= FILEATTR_DIRECTORY;
		}
		if(!(sb.st_mode & S_IWUSR)) {
			attr |= FILEATTR_READONLY;
		}
	}

	return(attr);
}

short file_dircreate(const char *path) {
	return((short)mkdir(path, 0777));
}

											// カレントファイル操作
void file_setcd(const char *exepath) {

	file_cpyname(curpath, exepath, sizeof(curpath));
	curfilep = file_getname(curpath);
	*curfilep = '\0';
}

char *file_getcd(const char *path) {

	*curfilep = '\0';
	file_catname(curpath, path, sizeof(curpath));
	return(curpath);
}

FILEH file_open_c(const char *path) {

	*curfilep = '\0';
	file_catname(curpath, path, sizeof(curpath));
	return(file_open(curpath));
}

FILEH file_open_rb_c(const char *path) {

	*curfilep = '\0';
	file_catname(curpath, path, sizeof(curpath));
	return(file_open_rb(curpath));
}

FILEH file_create_c(const char *path) {

	*curfilep = '\0';
	file_catname(curpath, path, sizeof(curpath));
	return(file_create(curpath));
}

short file_delete_c(const char *path) {

	*curfilep = '\0';
	file_catname(curpath, path, sizeof(curpath));
	return(file_delete(curpath));
}

short file_attr_c(const char *path) {

	*curfilep = '\0';
	file_catname(curpath, path, sizeof(curpath));
	return(file_attr(curpath));
}

typedef struct {
	BOOL			eoff;
	FSIterator		fsi;
	FSCatalogInfo	fsci;
	HFSUniStr255	name;
} _FLHDL, *FLHDL;

static void char2str(char *dst, int size, const UniChar *uni, int unicnt) {

	CFStringRef	cfsr;

	cfsr = CFStringCreateWithCharacters(NULL, uni, unicnt);
	CFStringGetCString(cfsr, dst, size, CFStringGetSystemEncoding());
	CFRelease(cfsr);
}

FLISTH file_list1st(const char *dir, FLINFO *fli) {
	DIR		*ret;

	ret = opendir(dir);
	if (ret == NULL) {
		goto ff1_err;
	}
	if (file_listnext((FLISTH)ret, fli) == SUCCESS) {
		return((FLISTH)ret);
	}
	closedir(ret);

ff1_err:
	return(FLISTH_INVALID);
}

BOOL file_listnext(FLISTH hdl, FLINFO *fli) {
	struct dirent	*de;
	struct stat		sb;

	de = readdir((DIR *)hdl);
	if (de == NULL) {
		return(FAILURE);
	}
	if (fli) {
		memset(fli, 0, sizeof(*fli));
		fli->caps = FLICAPS_ATTR;
		fli->attr = (de->d_type & DT_DIR) ? FILEATTR_DIRECTORY : 0;

		if (stat(de->d_name, &sb) == 0) {
			fli->caps |= FLICAPS_SIZE;
			fli->size = (UINT)sb.st_size;
			if (S_ISDIR(sb.st_mode)) {
				fli->attr |= FILEATTR_DIRECTORY;
			}
			if (!(sb.st_mode & S_IWUSR)) {
				fli->attr |= FILEATTR_READONLY;
			}
			if (cnv_sttime(&sb.st_mtime, &fli->date, &fli->time) == SUCCESS) {
				fli->caps |= FLICAPS_DATE | FLICAPS_TIME;
			}
		}
		milstr_ncpy(fli->path, de->d_name, sizeof(fli->path));
	}
	return(SUCCESS);
}

void file_listclose(FLISTH hdl) {
	closedir((DIR *)hdl);
}

BOOL getLongFileName(char *dst, const char *path) {

	FSRef			fref;
	HFSUniStr255	name;

	if (*path == '\0') {
		return(false);
	}
	FSPathMakeRef((const UInt8*)path, &fref, NULL);
	if (FSGetCatalogInfo(&fref, kFSCatInfoNone, NULL, &name, NULL, NULL)
																!= noErr) {
		return(false);
	}
	char2str(dst, 512, name.unicode, name.length);
	if (!dst) {
		return(false);
	}
	return(true);
}


void file_catname(char *path, const char *name, int maxlen) {
	int		csize;
	
	while(maxlen > 0) {
		if (*path == '\0') {
			break;
		}
		path++;
		maxlen--;
	}
	file_cpyname(path, name, maxlen);
	while((csize = milstr_charsize(path)) != 0) {
		if ((csize == 1) && (*path == '/')) {
			*path = '/';
		}
		path += csize;
	}
}

char *file_getname(const char *path) {
	const OEMCHAR	*ret;
	int		csize;
	
	ret = path;
	while((csize = milstr_charsize(path)) != 0) {
		if ((csize == 1) && (*path == '/')) {
			ret = path + 1;
		}
		path += csize;
	}
	return((OEMCHAR *)ret);
}

void file_cutname(char *path) {

	char 	*p;

	p = file_getname(path);
	p[0] = '\0';
}

char *file_getext(const char *path) {

	char	*p;
	char	*q;

	p = file_getname(path);
	q = NULL;

	while(*p != '\0') {
		if (*p++ == '.') {
			q = p;
		}
	}
	if (!q) {
		q = p;
	}
	return(q);
}

void file_cutext(char *path) {

	char	*p;
	char	*q;

	p = file_getname(path);
	q = NULL;

	while(*p != '\0') {
		if (*p == '.') {
			q = p;
		}
		p++;
	}
	if (q) {
		*q = '\0';
	}
}

void file_cutseparator(char *path) {

	ssize_t		pos;

	pos = strlen(path) - 1;
	if ((pos > 0) && (path[pos] == ':')) {
		path[pos] = '\0';
	}
}

void file_setseparator(char *path, int maxlen) {

	ssize_t		pos;

	pos = strlen(path) - 1;
	if ((pos < 0) || (path[pos] == ':') || ((pos + 2) >= maxlen)) {
		return;
	}
	path[++pos] = ':';
	path[++pos] = '\0';
}

short file_rename(const OEMCHAR *existpath, const OEMCHAR *newpath) {
	return((short)rename(existpath, newpath));
}

short file_dirdelete(const OEMCHAR *path) {
	return((short)rmdir(path));
}
