#define FUSE_USE_VERSION 31
#include <fuse3/fuse.h>
#include <string>
#include <map>
#include <cstring>
#include <errno.h>

// "דיסק" וירטואלי המנוהל במרחב המשתמש
std::map<std::string, std::string> files;

// מימוש getattr - החזרת מטא-דאטה של קובץ (Inodes) [5]
static int do_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi) {
    (void) fi;
    memset(stbuf, 0, sizeof(struct stat));
    std::string p = path;

    if (p == "/") {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2; // refcount לספריה [6]
        return 0;
    } else if (files.count(p)) {
        stbuf->st_mode = S_IFREG | 0666;
        stbuf->st_nlink = 1;
        stbuf->st_size = files[p].size();
        return 0;
    }
    return -ENOENT;
}

static int do_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi, enum fuse_readdir_flags flags) {
    (void) offset; (void) fi; (void) flags;
    filler(buf, ".", NULL, 0, (fuse_fill_dir_flags)0);
    filler(buf, "..", NULL, 0, (fuse_fill_dir_flags)0);

    for (std::map<std::string, std::string>::iterator it = files.begin(); it != files.end(); ++it) {
        filler(buf, it->first.c_str() + 1, NULL, 0, (fuse_fill_dir_flags)0);
    }
    return 0;
}

static int do_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    (void) mode; (void) fi;
    if (files.count(path)) return -EEXIST;
    files[path] = ""; 
    return 0;
}

static int do_unlink(const char *path) {
    if (files.count(path)) {
        files.erase(path); // מחיקת הקובץ וה-Inode מהמפה [7]
        return 0;
    }
    return -ENOENT;
}

static int do_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    (void) fi;
    if (!files.count(path)) return -ENOENT;
    const std::string& content = files[path];
    if (offset >= (off_t)content.size()) return 0;
    if (offset + size > content.size()) size = content.size() - offset;
    memcpy(buf, content.data() + offset, size);
    return size;
}

static int do_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    (void) fi;
    if (!files.count(path)) return -ENOENT;
    std::string& content = files[path];
    if (offset + size > content.size()) content.resize(offset + size);
    memcpy(&content[offset], buf, size);
    return size;
}

static int do_truncate(const char *path, off_t size, struct fuse_file_info *fi) {
    (void) fi;
    if (files.count(path)) { files[path].resize(size); return 0; }
    return -ENOENT;
}

// שימוש במבנה סטטי ואתחול ידני למניעת בעיות C++20
static struct fuse_operations my_ops;

int main(int argc, char *argv[]) {
    memset(&my_ops, 0, sizeof(struct fuse_operations));
    my_ops.getattr = do_getattr;
    my_ops.readdir = do_readdir;
    my_ops.create  = do_create;
    my_ops.unlink  = do_unlink;
    my_ops.read    = do_read;
    my_ops.write   = do_write;
    my_ops.truncate = do_truncate;

    return fuse_main(argc, argv, &my_ops, NULL);
}
