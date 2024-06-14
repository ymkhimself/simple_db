#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

/**
 * 一个REPL程序 read eval print loop
 * 读取 解析 执行
 *
 */

/**
 * 如何储存数据
 * 1. 将数据行存储在内存块中，内存块即pages
 * 2. 一个page尽可能多地储存数据行
 * 3. 数据行被压缩之后存到page中
 * 4. page只有在需要的时候才分配
 * 5. 通过一个定长的数组来组织page(sqlite使用B-tree)
 */

// 牛逼，编译时确定结构体某个属性的大小(字节数)
#define size_of_attribute(Struct, Attribute) sizeof(((Struct *)0)->Attribute)

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

const uint32_t ID_SIZE = size_of_attribute(Row, id);
const uint32_t USERNAME_SIZE = size_of_attribute(Row, username);
const uint32_t EMAIL_SIZE = size_of_attribute(Row, email);
const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;
const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;

// 内存拷贝 帅
void serialize_row(Row *source, void *destination) {
    memcpy(destination + ID_OFFSET, &(source->id), ID_SIZE);
    memcpy(destination + USERNAME_SIZE, &(source->username), USERNAME_SIZE);
    memcpy(destination + EMAIL_OFFSET, &(source->email), EMAIL_SIZE);
}

void deserialize_row(void *source, Row *destination) {
    memcpy(&(destination->id), source + ID_OFFSET, ID_SIZE);
    memcpy(&destination->username, source + USERNAME_OFFSET, USERNAME_SIZE);
    memcpy(&destination->email, source + EMAIL_OFFSET, EMAIL_SIZE);
}

// 表
const uint32_t PAGE_SIZE =
    4096; // 页大小，固定  4kb 大部分OS
          // 一个虚拟内存页都是4kb,所以这样的OS调出掉入page时，是一整个来操作，不会分开它
#define TABLE_MAX_PAGES 100                          // 页数量 固定
const uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE; // 每一页最多多少行
const uint32_t TABLE_MAX_ROWS =
    ROWS_PER_PAGE * TABLE_MAX_PAGES; // 表中最多多少行

typedef struct {
    uint32_t num_rows; // 指出有多少行
    void *pages[TABLE_MAX_PAGES];
} Table;

// 根据row_num拿到ROW
void *row_slot(Table *table, uint32_t row_num) {
    uint32_t page_num = row_num / ROWS_PER_PAGE; // 不需要+1，因为数组从0开始
    void *page = table->pages[page_num];
    if (page == NULL) {
        page = table->pages[page_num] = malloc(PAGE_SIZE);
    }
    uint32_t row_offset = row_num % ROWS_PER_PAGE;
    uint32_t byte_offset = row_offset * ROW_SIZE;
    return page + byte_offset;
}

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
                                Statement *statement) {
    // 读取前6个字符，如果是insert
    if (strncmp(inpute_buffer->buffer, "insert", 6)) {
        statement->type = STATEMENT_INSERT;
        int args_assigned = sscanf(inpute_buffer->buffer, "insert %d %d %s %s",
                                   &(statement->row_to_insert.id),
                                   statement->row_to_insert.username,
                                   statement->row_to_insert.email);
        // 没有读到预期数量的参数，返回语法错误
        if (args_assigned < 3) {
            return PREPARE_SYNTAX_ERROR;
        }
    }
    if (strcmp(inpute_buffer->buffer, "insert") == 0) {
    }
}

ExecuteResult execute_insert(Statement *statement, Table *table) {}

ExecuteResult execute_select(Statement *statement, Table *table) {}

// 执行语句
ExecuteResult execute_statement(Statement *statement, Table *table) {
    switch (statement->type) {
    case (STATEMENT_INSERT):
        return execute_insert(statement, table);
    case (STATEMENT_SELECT):
        return execute_select(statement, table);
    }
}

int main() {
    Table *table;
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
                printf("Unrecongnized command '%s'\n", input_buffer->buffer);
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
        execute_statement(&statement, table);
        printf("Executed.\n");
    }
}