# VSFS TESTS

Several of the commands share the same tests and are hence omitted.

## Setup
-`make clean`
- `make`
- `cd tests`


## `vsfs list`

- Invalid number of arguments (1 < or > 1).\
  Command - `../vsfs list`/`../vsfs list FS_default.notes EF_default`\
  Output - 
  - Invalid VSFS: Arguments for command "list", expected 1, received 0 (errno 1)
  - Invalid VSFS: Arguments for command "list", expected 1, received 2 (errno 1)


- The specified FS does not exist.\
  Command - `../vsfs list non_existent.notes`\
  Output - Invalid VSFS: FS could not be found non_existent.notes (errno 2 ENOENT)


- FS does not have the .notes or .gz extension.\
  Command - `../vsfs list invalid_extension.exe`\
  Output - Invalid VSFS: FS must end with the ".notes" or ".gz" extension, unrecognised extension: exe (errno 1)


- FS does not have the first record.\
  Command - `../vsfs list missing_first_record.notes`\
  Output - Invalid VSFS: First record of FS must be "NOTES V1.0" (errno 1)


- FS has record type other than '@'/'='/' '/'#'.\
  Command - `../vsfs list invalid_record_type.notes`\
  Output - Invalid VSFS: Unknown record type $ (errno 1)


- FS has invalid file/dir record name, e.g.,\
     * Not beginning/containing '..'\
     * Not beginning/containing '.'\
     * Not beginning with a '/'\
     * Not ending with a '/' for files, ending with a '/' for dirs.\
  Command - `../vsfs list invalid_record_name.notes`\
  Output - Invalid VSFS: Invalid record path "dir1/file2/" (errno 1)


- FS has duplicate file/dir records in the same level.\
  Command - `../vsfs list duplicate_file_records.notes`/`./vsfs list duplicate_dir_records.notes`\
  Output 
  - Invalid VSFS: FS file "file1" already exists in dir "dir1/" (errno 1)
  - Invalid VSFS: FS dir "dir1/dir2/" already exists in dir "dir1/" (errno 1)


- FS permissions are captured properly (varying permissions using chmod).\
  Command - `../vsfs list less_permitted.notes`\
  Output - ... -r-xr-xr-x ... (errno 0)


- Zipped FS can be read and be restored/re-zipped once read.\
  Command - `../vsfs list zipped.notes.gz`\
  Output - Standard list output with the .gz file intact (errno 0)


- No memory leaks.\
  Command - `./run_through_valgrind.sh list zipped.notes.gz`\
  Output - 
    ... total heap usage: 172 allocs, 172 frees, 122,963 bytes allocated ... (errno 0)

## `vsfs copyin`

- EF does not exist.\
  Command - `../vsfs copyin FS_default.notes EF_non_existent IF_non_existent`\
  Output - Invalid VSFS: EF could not be found: EF_non_existent (errno 0)


- Copy in ASCII file with content records.\
  Command - `../vsfs copyin FS_default.notes EF_default IF_default`\
  Output - Record successfully added to FS (errno 0)


- Encode and copy in non-ASCII file.\
  Command - 
  - `dd if=/dev/zero of=EF_binary.bin bs=16 count=0 seek=1024 && 
    ../vsfs copyin FS_default.notes EF_binary.bin IF_binary`
  - `../vsfs copyin FS_default.notes ../vsfs IF_vsfs`\
  Output 
  - Binary file successfully encoded and added to FS (errno 0)
  - Non-ASCII file successfully encoded and added to FS (errno 0)


- Invalid IF path given.\
  Command - `../vsfs copyin FS_default.notes EF_default ./IF_invalid`\
  Output - Invalid VSFS: Invalid IF provided "./IF_invalid" (errno 1)


- Records having length greater than 255 truncated.
  Command - `../vsfs copyin FS_default.notes EF_large IF_truncated`\
  Output - Record is truncated at length 255 (errno 0)


- EF is empty.
  Command - `../vsfs copyin FS_default.notes EF_empty IF_empty`\
  Output - Empty record added, verified by adding a second record (errno 0)


- IF already exists and hence deleted before appending.
  Command - `../vsfs copyin FS_default.notes EF_empty IF_existing
  && ../vsfs copyin FS_default.notes EF_default IF_existing`\
  Output - Two records added, the former was deleted to indicate replacing the record (errno 0)


- Intermediate IF directories created.
  Command
  - `../vsfs copyin FS_default.notes EF_default test_dir1/IF_intermediate`\
  - `../vsfs copyin FS_default.notes EF_default test_dir1/test_dir2/test_dir3/IF_intermediate`\
  Output - All intermediate directories added (errno 0)
  Output - Only the non-existent intermediate directories added (errno 0)

## `vsfs copyout`

- IF does not exist.\
  Command - `../vsfs copyout FS_default.notes IF_non_existent EF_non_existent`\
  Output - Invalid VSFS: IF could not be found "IF_non_existent" (errno 2)


- Copy out ASCII file with content records.\
  Command - `../vsfs copyout FS_default.notes IF_default EF_out`
  Output - Invalid VSFS: IF could not be found "IF_non_existent" (errno 0)


- Copy out and decode non-ASCII file.
  Command - `../vsfs copyout FS_default.notes IF_vsfs new_vsfs`\
  Output - Binary file successfully output and decoded into new executable (errno 0)


- EF already exists and hence deleted before appending.
  Command - `../vsfs copyout FS_default.notes IF_default EF_default`\
  Output - EF's content erased and new content inserted (errno 0)

## `vsfs mkdir`

- The '/' at the end of ID name may be optional. Ensure this stays consistent in FS.
  Command - `../vsfs mkdir FS_default.notes ID_default`\
  Output - Dir created with name "ID_default/" (errno 0)


- ID already exists and hence error is thrown.
  Command - `../vsfs mkdir FS_default.notes ID_existing
  && ../vsfs mkdir FS_default.notes ID_existing`\
  Output - Invalid VSFS: ID already exists "ID_existing/" (errno 0)


## `vsfs rm`

- Delete IF with content records.
  Command - `../vsfs rm FS_default.notes IF_default`\
  Output - Record successfully deleted (errno 0)


- IF does not exist.\
  Command - `../vsfs rm FS_default.notes IF_non_existent`\
  Output - Invalid VSFS: IF could not be found "IF_non_existent" (errno 2)


- IF exists but is deleted and hence not found.
  Command - `../vsfs rm FS_default.notes IF_deleted`\
  Output - Invalid VSFS: IF could not be found "IF_deleted" (errno 2)


## `vsfs rmdir`

- ID does not exist.\
  Command - `../vsfs rmdir FS_default.notes ID_non_existent`\
  Output - Invalid VSFS: ID could not be found "ID_non_existent/" (errno 2)


- ID exists but is deleted and hence not found.
  Command - `../vsfs rmdir FS_default.notes ID_deleted`\
  Output - Invalid VSFS: ID could not be found "ID_deleted" (errno 2)

  
## `vsfs defrag`

- Deleted records are discarded in the defragged FS.
  Command - `../vsfs defrag FS_default.notes`\
  Output - Fully defragged file with no deleted records (errno 0)


- Records are sorted according to their alphabetical order. 
  Directories are given higher privilege as in notes file, the dir record must exist before any record within that dir.
  Command - `../vsfs defrag FS_default.notes`\
  Output - New FS is sorted according to the criteria. Dirs appear before their children. (errno 0)
