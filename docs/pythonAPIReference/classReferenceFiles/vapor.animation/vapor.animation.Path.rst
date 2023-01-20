.. _vapor.animation.Path:


vapor.animation.Path
--------------------


Help on class Path in vapor.animation:

vapor.animation.Path = class Path(PurePath)
 |  vapor.animation.Path(*args, **kwargs)
 |  
 |  PurePath subclass that can make system calls.
 |  
 |  Path represents a filesystem path but unlike PurePath, also offers
 |  methods to do system calls on path objects. Depending on your system,
 |  instantiating a Path will return either a PosixPath or a WindowsPath
 |  object. You can also instantiate a PosixPath or WindowsPath directly,
 |  but cannot instantiate a WindowsPath on a POSIX system or vice versa.
 |  
 |  Method resolution order:
 |      Path
 |      PurePath
 |      builtins.object
 |  
 |  Methods defined here:
 |  
 |  __enter__(self)
 |  
 |  __exit__(self, t, v, tb)
 |  
 |  absolute(self)
 |      Return an absolute version of this path.  This function works
 |      even if the path doesn't point to anything.
 |      
 |      No normalization is done, i.e. all '.' and '..' will be kept along.
 |      Use resolve() to get the canonical path to a file.
 |  
 |  chmod(self, mode)
 |      Change the permissions of the path, like os.chmod().
 |  
 |  exists(self)
 |      Whether this path exists.
 |  
 |  expanduser(self)
 |      Return a new path with expanded ~ and ~user constructs
 |      (as returned by os.path.expanduser)
 |  
 |  glob(self, pattern)
 |      Iterate over this subtree and yield all existing files (of any
 |      kind, including directories) matching the given relative pattern.
 |  
 |  group(self)
 |      Return the group name of the file gid.
 |  
 |  is_block_device(self)
 |      Whether this path is a block device.
 |  
 |  is_char_device(self)
 |      Whether this path is a character device.
 |  
 |  is_dir(self)
 |      Whether this path is a directory.
 |  
 |  is_fifo(self)
 |      Whether this path is a FIFO.
 |  
 |  is_file(self)
 |      Whether this path is a regular file (also True for symlinks pointing
 |      to regular files).
 |  
 |  is_mount(self)
 |      Check if this path is a POSIX mount point
 |  
 |  is_socket(self)
 |      Whether this path is a socket.
 |  
 |  is_symlink(self)
 |      Whether this path is a symbolic link.
 |  
 |  iterdir(self)
 |      Iterate over the files in this directory.  Does not yield any
 |      result for the special paths '.' and '..'.
 |  
 |  lchmod(self, mode)
 |      Like chmod(), except if the path points to a symlink, the symlink's
 |      permissions are changed, rather than its target's.
 |  
 |  link_to(self, target)
 |      Make the target path a hard link pointing to this path.
 |      
 |      Note this function does not make this path a hard link to *target*,
 |      despite the implication of the function and argument names. The order
 |      of arguments (target, link) is the reverse of Path.symlink_to, but
 |      matches that of os.link.
 |  
 |  lstat(self)
 |      Like stat(), except if the path points to a symlink, the symlink's
 |      status information is returned, rather than its target's.
 |  
 |  mkdir(self, mode=511, parents=False, exist_ok=False)
 |      Create a new directory at this given path.
 |  
 |  open(self, mode='r', buffering=-1, encoding=None, errors=None, newline=None)
 |      Open the file pointed by this path and return a file object, as
 |      the built-in open() function does.
 |  
 |  owner(self)
 |      Return the login name of the file owner.
 |  
 |  read_bytes(self)
 |      Open the file in bytes mode, read it, and close the file.
 |  
 |  read_text(self, encoding=None, errors=None)
 |      Open the file in text mode, read it, and close the file.
 |  
 |  readlink(self)
 |      Return the path to which the symbolic link points.
 |  
 |  rename(self, target)
 |      Rename this path to the target path.
 |      
 |      The target path may be absolute or relative. Relative paths are
 |      interpreted relative to the current working directory, *not* the
 |      directory of the Path object.
 |      
 |      Returns the new Path instance pointing to the target path.
 |  
 |  replace(self, target)
 |      Rename this path to the target path, overwriting if that path exists.
 |      
 |      The target path may be absolute or relative. Relative paths are
 |      interpreted relative to the current working directory, *not* the
 |      directory of the Path object.
 |      
 |      Returns the new Path instance pointing to the target path.
 |  
 |  resolve(self, strict=False)
 |      Make the path absolute, resolving all symlinks on the way and also
 |      normalizing it (for example turning slashes into backslashes under
 |      Windows).
 |  
 |  rglob(self, pattern)
 |      Recursively yield all existing files (of any kind, including
 |      directories) matching the given relative pattern, anywhere in
 |      this subtree.
 |  
 |  rmdir(self)
 |      Remove this directory.  The directory must be empty.
 |  
 |  samefile(self, other_path)
 |      Return whether other_path is the same or not as this file
 |      (as returned by os.path.samefile()).
 |  
 |  stat(self)
 |      Return the result of the stat() system call on this path, like
 |      os.stat() does.
 |  
 |  symlink_to(self, target, target_is_directory=False)
 |      Make this path a symlink pointing to the target path.
 |      Note the order of arguments (link, target) is the reverse of os.symlink.
 |  
 |  touch(self, mode=438, exist_ok=True)
 |      Create this file with the given access mode, if it doesn't exist.
 |  
 |  unlink(self, missing_ok=False)
 |      Remove this file or link.
 |      If the path is a directory, use rmdir() instead.
 |  
 |  write_bytes(self, data)
 |      Open the file in bytes mode, write to it, and close the file.
 |  
 |  write_text(self, data, encoding=None, errors=None)
 |      Open the file in text mode, write to it, and close the file.
 |  
 |  ----------------------------------------------------------------------
 |  Class methods defined here:
 |  
 |  cwd() from builtins.type
 |      Return a new path pointing to the current working directory
 |      (as returned by os.getcwd()).
 |  
 |  home() from builtins.type
 |      Return a new path pointing to the user's home directory (as
 |      returned by os.path.expanduser('~')).
 |  
 |  ----------------------------------------------------------------------
 |  Static methods defined here:
 |  
 |  __new__(cls, *args, **kwargs)
 |      Construct a PurePath from one or several strings and or existing
 |      PurePath objects.  The strings and path objects are combined so as
 |      to yield a canonicalized path, which is incorporated into the
 |      new PurePath object.
 |  
 |  ----------------------------------------------------------------------
 |  Methods inherited from PurePath:
 |  
 |  __bytes__(self)
 |      Return the bytes representation of the path.  This is only
 |      recommended to use under Unix.
 |  
 |  __eq__(self, other)
 |      Return self==value.
 |  
 |  __fspath__(self)
 |  
 |  __ge__(self, other)
 |      Return self>=value.
 |  
 |  __gt__(self, other)
 |      Return self>value.
 |  
 |  __hash__(self)
 |      Return hash(self).
 |  
 |  __le__(self, other)
 |      Return self<=value.
 |  
 |  __lt__(self, other)
 |      Return self<value.
 |  
 |  __reduce__(self)
 |      Helper for pickle.
 |  
 |  __repr__(self)
 |      Return repr(self).
 |  
 |  __rtruediv__(self, key)
 |  
 |  __str__(self)
 |      Return the string representation of the path, suitable for
 |      passing to system calls.
 |  
 |  __truediv__(self, key)
 |  
 |  as_posix(self)
 |      Return the string representation of the path with forward (/)
 |      slashes.
 |  
 |  as_uri(self)
 |      Return the path as a 'file' URI.
 |  
 |  is_absolute(self)
 |      True if the path is absolute (has both a root and, if applicable,
 |      a drive).
 |  
 |  is_relative_to(self, *other)
 |      Return True if the path is relative to another path or False.
 |  
 |  is_reserved(self)
 |      Return True if the path contains one of the special names reserved
 |      by the system, if any.
 |  
 |  joinpath(self, *args)
 |      Combine this path with one or several arguments, and return a
 |      new path representing either a subpath (if all arguments are relative
 |      paths) or a totally different path (if one of the arguments is
 |      anchored).
 |  
 |  match(self, path_pattern)
 |      Return True if this path matches the given pattern.
 |  
 |  relative_to(self, *other)
 |      Return the relative path to another path identified by the passed
 |      arguments.  If the operation is not possible (because this is not
 |      a subpath of the other path), raise ValueError.
 |  
 |  with_name(self, name)
 |      Return a new path with the file name changed.
 |  
 |  with_stem(self, stem)
 |      Return a new path with the stem changed.
 |  
 |  with_suffix(self, suffix)
 |      Return a new path with the file suffix changed.  If the path
 |      has no suffix, add given suffix.  If the given suffix is an empty
 |      string, remove the suffix from the path.
 |  
 |  ----------------------------------------------------------------------
 |  Class methods inherited from PurePath:
 |  
 |  __class_getitem__(type) from builtins.type
 |  
 |  ----------------------------------------------------------------------
 |  Readonly properties inherited from PurePath:
 |  
 |  anchor
 |      The concatenation of the drive and root, or ''.
 |  
 |  drive
 |      The drive prefix (letter or UNC path), if any.
 |  
 |  name
 |      The final path component, if any.
 |  
 |  parent
 |      The logical parent of the path.
 |  
 |  parents
 |      A sequence of this path's logical parents.
 |  
 |  parts
 |      An object providing sequence-like access to the
 |      components in the filesystem path.
 |  
 |  root
 |      The root of the path, if any.
 |  
 |  stem
 |      The final path component, minus its last suffix.
 |  
 |  suffix
 |      The final component's last suffix, if any.
 |      
 |      This includes the leading period. For example: '.txt'
 |  
 |  suffixes
 |      A list of the final component's suffixes, if any.
 |      
 |      These include the leading periods. For example: ['.tar', '.gz']

