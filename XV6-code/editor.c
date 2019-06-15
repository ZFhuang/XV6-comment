#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "fs.h"

// ************************************************************ const strings *
#define MAX_CHAR_INSHOW 80
#define BUF_LINE_LENGTH 320

// *********************************************************** data structure *
// use linkedlist to store the text
typedef struct LineNode {
	struct LineNode* prev;
	struct LineNode* next;
	int length;
	char* content;
}LineNode;

// **************************************************** function decalrations *
int loadFile(char* filePath);
void showFile();
void initList();
LineNode* newNode(LineNode* prev);
char* setStr(char* target, char* from, int len);

// **************************************************************** variables *
int fileLineNum = 0;
LineNode* headNode;
LineNode* tailNode;

// *********************************************************** function bodys *
int main(int argc, char* argv[])
{
	if (argc == 1) {
		// if missing some parameters
		printf(1, "Please start this editor using [editor file_name]\n");
		exit();
		return -1;
	}

	if (loadFile(argv[1])) {
		// check successfully
		printf(1, "File lines: %d", fileLineNum);
	}
	else {
		// something goes wrong
		exit();
		return -1;
	}

	showFile();

	// editor ends normally
	exit();
	return 0;
}

// load the file into first linkedlist
int loadFile(char* filePath) {
	// check if [filePath] is exist
	int file = open(filePath, O_RDONLY);
	if (file == -1) {
		printf(1, "File not found!\n");
		return 0;
	}
	printf(1, "File found!\n");

	// init it
	initList();

	// the most important part
	// adding every lines into that linkedlist
	char buf[BUF_LINE_LENGTH] = {};
	int len = 0;
	int start = 0;
	int end = 0;
	int scaning = 0;
	LineNode* now;

	// read a new line
	while ((len = read(file, buf, BUF_LINE_LENGTH))) {
		// if the [start pointer] is still inline
		while (start < len) {
			// find out the line's ending
			for (end = start; end < len && buf[end] != '\n'; ++end)
				;

			// [scaning == 1] means is still reading the latest line
			if (scaning == 0) {
				now = newNode(tailNode->prev);
			}

			// if is a new line
			if (now->length == 0) {
				now->content = (char*)malloc(sizeof(char)*now->length);
				setStr(&now->content[0], &buf[start], end - start);
				now->length = 0 + end - start;
			}
			// if is a old line
			else {
				char* tempStr = now->content;
				now->content = (char*)malloc(sizeof(char)*(now->length
					+ end - start));
				setStr(&now->content[0], &tempStr[0], now->length);
				setStr(&now->content[now->length], &buf[start], end - start);
				now->length = now->length + end - start;
			}

			// if meets a line's ending
			if (buf[end] == '\n') {
				scaning = 0;
				fileLineNum++;
			}
			// this line is not end now
			else {
				scaning = 1;
			}

			// continue
			start = end + 1;
		}
	}

	return 1;
}

// display file to commandline
void showFile() {
	;
}

// init linkedlist
void initList() {
	tailNode = (LineNode*)malloc(sizeof(LineNode));
	headNode = (LineNode*)malloc(sizeof(LineNode));

	tailNode->prev = headNode;
	tailNode->next = 0;
	headNode->prev = 0;
	headNode->next = tailNode;
}

// add a new node after [prev] and return it
LineNode* newNode(LineNode* prev) {
	LineNode* node;
	node = (LineNode*)malloc(sizeof(LineNode));
	LineNode* next = prev->next;
	if (next) {
		next->prev = node;
		prev->next = node;
		node->prev = prev;
		node->next = next;
	}
	else {
		prev->next = node;
		node->prev = prev;
		node->next = 0;
	}

	return node;
}

// parse [from+len] to target
char* setStr(char* target, char* from, int len) {
	int i = 0;
	for (i = 0; i < len; ++i) {
		target[i] = from[i];
	}

	return target;
}
