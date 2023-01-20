.. _vapor.animation.BytesIO:


vapor.animation.BytesIO
-----------------------


Help on class BytesIO in vapor.animation:

vapor.animation.BytesIO = class BytesIO(_BufferedIOBase)
 |  vapor.animation.BytesIO(initial_bytes=b'')
 |  
 |  Buffered I/O implementation using an in-memory bytes buffer.
 |  
 |  Method resolution order:
 |      BytesIO
 |      _BufferedIOBase
 |      _IOBase
 |      builtins.object
 |  
 |  Methods defined here:
 |  
 |  __getstate__(...)
 |  
 |  __init__(self, /, *args, **kwargs)
 |      Initialize self.  See help(type(self)) for accurate signature.
 |  
 |  __iter__(self, /)
 |      Implement iter(self).
 |  
 |  __next__(self, /)
 |      Implement next(self).
 |  
 |  __setstate__(...)
 |  
 |  __sizeof__(...)
 |      Size of object in memory, in bytes.
 |  
 |  close(self, /)
 |      Disable all I/O operations.
 |  
 |  flush(self, /)
 |      Does nothing.
 |  
 |  getbuffer(self, /)
 |      Get a read-write view over the contents of the BytesIO object.
 |  
 |  getvalue(self, /)
 |      Retrieve the entire contents of the BytesIO object.
 |  
 |  isatty(self, /)
 |      Always returns False.
 |      
 |      BytesIO objects are not connected to a TTY-like device.
 |  
 |  read(self, size=-1, /)
 |      Read at most size bytes, returned as a bytes object.
 |      
 |      If the size argument is negative, read until EOF is reached.
 |      Return an empty bytes object at EOF.
 |  
 |  read1(self, size=-1, /)
 |      Read at most size bytes, returned as a bytes object.
 |      
 |      If the size argument is negative or omitted, read until EOF is reached.
 |      Return an empty bytes object at EOF.
 |  
 |  readable(self, /)
 |      Returns True if the IO object can be read.
 |  
 |  readinto(self, buffer, /)
 |      Read bytes into buffer.
 |      
 |      Returns number of bytes read (0 for EOF), or None if the object
 |      is set not to block and has no data to read.
 |  
 |  readline(self, size=-1, /)
 |      Next line from the file, as a bytes object.
 |      
 |      Retain newline.  A non-negative size argument limits the maximum
 |      number of bytes to return (an incomplete line may be returned then).
 |      Return an empty bytes object at EOF.
 |  
 |  readlines(self, size=None, /)
 |      List of bytes objects, each a line from the file.
 |      
 |      Call readline() repeatedly and return a list of the lines so read.
 |      The optional size argument, if given, is an approximate bound on the
 |      total number of bytes in the lines returned.
 |  
 |  seek(self, pos, whence=0, /)
 |      Change stream position.
 |      
 |      Seek to byte offset pos relative to position indicated by whence:
 |           0  Start of stream (the default).  pos should be >= 0;
 |           1  Current position - pos may be negative;
 |           2  End of stream - pos usually negative.
 |      Returns the new absolute position.
 |  
 |  seekable(self, /)
 |      Returns True if the IO object can be seeked.
 |  
 |  tell(self, /)
 |      Current file position, an integer.
 |  
 |  truncate(self, size=None, /)
 |      Truncate the file to at most size bytes.
 |      
 |      Size defaults to the current file position, as returned by tell().
 |      The current file position is unchanged.  Returns the new size.
 |  
 |  writable(self, /)
 |      Returns True if the IO object can be written.
 |  
 |  write(self, b, /)
 |      Write bytes to file.
 |      
 |      Return the number of bytes written.
 |  
 |  writelines(self, lines, /)
 |      Write lines to the file.
 |      
 |      Note that newlines are not added.  lines can be any iterable object
 |      producing bytes-like objects. This is equivalent to calling write() for
 |      each element.
 |  
 |  ----------------------------------------------------------------------
 |  Static methods defined here:
 |  
 |  __new__(*args, **kwargs) from builtins.type
 |      Create and return a new object.  See help(type) for accurate signature.
 |  
 |  ----------------------------------------------------------------------
 |  Data descriptors defined here:
 |  
 |  closed
 |      True if the file is closed.
 |  
 |  ----------------------------------------------------------------------
 |  Methods inherited from _BufferedIOBase:
 |  
 |  detach(self, /)
 |      Disconnect this buffer from its underlying raw stream and return it.
 |      
 |      After the raw stream has been detached, the buffer is in an unusable
 |      state.
 |  
 |  readinto1(self, buffer, /)
 |  
 |  ----------------------------------------------------------------------
 |  Methods inherited from _IOBase:
 |  
 |  __del__(...)
 |  
 |  __enter__(...)
 |  
 |  __exit__(...)
 |  
 |  fileno(self, /)
 |      Returns underlying file descriptor if one exists.
 |      
 |      OSError is raised if the IO object does not use a file descriptor.
 |  
 |  ----------------------------------------------------------------------
 |  Data descriptors inherited from _IOBase:
 |  
 |  __dict__

