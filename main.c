#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

// 输入缓冲
typedef struct main {
    char *buffer;
    size_t buffer_length;
    ssize_t input_length;
} InputBuffer;

typedef enum { STATEMENT_INSERT, STATEMENT_SELECT } StatementType;

// 基本命令结果
typedef enum {
    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECOGNIZED_COMMAND
} MetaCommandResult;

// prepare的结果
typedef enum {
    PREPARE_SUCCESS,
    PREPARE_SYNTAX_ERROR,
    PREPARE_UNRECOGNIZED_STATEMENT
} PrepareResult;

// 执行结果
typedef enum { EXECUTE_SUCCESS, EXECUTE_TABLE_FULL } ExecuteResult;

// 行
#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255
typedef struct {
    uint32_t id;
    char username[COLUMN_USERNAME_SIZE];
    char email[COLUMN_EMAIL_SIZE];
} Row;

// 语句
typedef struct {
    StatementType type;
    Row row_to_insert; // 只用于插入语句
} Statement;

#define size_of_attribute(Struct, Attribute) sizeof(((Struct *)0)->Attribute)

InputBuffer *new_input_buffer() {
    InputBuffer *input_buffer = malloc(sizeof(InputBuffer));
    input_buffer->buffer = NULL;
    input_buffer->buffer_length = 0;
    input_buffer->input_length = 0;
}

// 读取输入
void read_input(InputBuffer *input_buffer) {
    ssize_t bytes_read =
        getline(&(input_buffer->buffer), &(input_buffer->buffer_length), stdin);
    if (bytes_read < 0) {
        printf("Error readint input\n");
        exit(EXIT_FAILURE);
    }

    // 忽略行末的换行符
    input_buffer->input_length = bytes_read - 1;
    input_buffer->buffer[bytes_read - 1] = 0;
}

// 关闭input_buffer
void close_input_buffer(InputBuffer *input_buffer) {
    free(input_buffer->buffer);
    free(input_buffer);
}

void print_prompt() { printf("db > "); }

// 执行基本命令
MetaCommandResult do_meta_command(InputBuffer *input_buffer) {}

// 预处理语句
PrepareResult prepare_statement(InputBuffer *inpute_buffer,
                                Statement *statement) {}

// 执行语句
void execute_statement(Statement *statement) {}

int main() {
    InputBuffer *input_buffer = new_input_buffer();
    while (true) {
        print_prompt();
        read_input(input_buffer);
        // 基本命令以 "." 开头
        if (input_buffer->buffer[0] == '.') {
            switch (do_meta_command(input_buffer)) {
            case (META_COMMAND_SUCCESS):
                continue;
            case (META_COMMAND_UNRECOGNIZED_COMMAND):
                print("Unrecongnized command '%s'\n", input_buffer->buffer);
                continue;
            }
        }

        Statement statement;
        switch (prepare_statement(input_buffer, &statement)) {
        case (PREPARE_SUCCESS):
            break;
        case (PREPARE_UNRECOGNIZED_STATEMENT):
            printf("Unrecognized keyword at start of '%s' .\n",
                   input_buffer->buffer);
            continue;
        }
        execute_statement(&statement);
        printf("Executed.\n");
    }
}