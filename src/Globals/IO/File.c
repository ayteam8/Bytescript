#include "Globals/IO/File.h"

TCDataType FileData_Type = { .name = "FileData" }, *FileDataType = &FileData_Type;

TValue File_class;
void initialize_file_class() {
    TValue File_fields[] = {
        new_cfunc(read_file),
        new_cfunc(write_file),
        new_cfunc(close_file)
    };

    File_class = new_class(NULL, File_fields, 0, sizeof(File_fields) / sizeof(TValue));
    add_global(File_class, make_fstring("File"));
}


TValue read_file(TValue* argv, size_t argc) {
    MIN_ARGC(1);

    TObject* obj = AS_OBJECT(argv[0]);
    TValue tfile = obj->fields[0];

    FileData* file = as_filedata(tfile);

    fseek(file->handle, 0, SEEK_END);

    long size = ftell(file->handle);
    if (size == -1) call_error(new_failed_allocation_error, "Failed allocation while reading file");
    
    rewind(file->handle);

    TStr* str = malloc(sizeof(TStr) + size + 1);
    if (!str) call_error(new_failed_allocation_error, "Failed allocation of buffer while reading file");

    size_t read = fread(str->str, 1, size, file->handle);
    str->str[read] = '\0';

    fclose(file->handle);
    file->handle = NULL;
    return STR_TO_TVALUE(str);
}

TValue write_file(TValue* argv, size_t argc) {
    MIN_ARGC(2);
    if (TYPE(argv[0]) != OBJECT || TYPE(argv[1]) != STRING) call_error(new_type_error, "function <File.write> take types <File> and <String> as arguments");

    TObject* obj = AS_OBJECT(argv[0]);
    TStr* content = AS_STRING(argv[1]);
    TValue tfile = obj->fields[0];

    FileData* file = as_filedata(tfile);
    fwrite(content->str, 1, content->len, file->handle);
    
    return argv[1];
}

TValue close_file(TValue* argv, size_t argc) {
    MIN_ARGC(1);
    if (TYPE(argv[0]) != OBJECT) call_error(new_type_error, "function <File.close> take type <File> as argument");

    TObject* obj = AS_OBJECT(argv[0]);
    TStr* content = AS_STRING(argv[1]);
    TValue tfile = obj->fields[0];

    FileData* file = as_filedata(tfile);
    if (file->handle==NULL) return NIL;
    fclose(file->handle);

    return NIL;
}


TValue new_filedata(FILE* pfile) {
    FileData* file_data = malloc(sizeof(FileData));
    file_data->cdata_type = FileDataType;
    file_data->handle = pfile;

    return CDT_TO_TVALUE((TCData*)file_data);
}

FileData* as_filedata(TValue value) {
    TCData* filedata = AS_CDATA(value);
    if (!(*filedata == FileDataType)) call_error(new_type_error, "Invalid FileData in <File> object. in function <File.write>");

    return (FileData*)filedata;
}
