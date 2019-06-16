#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "fs.h"

// ************************************************************ const strings *
#define MAX_CHAR_INSHOW 80
#define BUF_LINE_LENGTH 320
#define NULL 0

// *********************************************************** data structure *
// use linkedlist to store the text
typedef struct LineNode {
	struct LineNode* prev;
	int length;
	char* content;
	struct LineNode* next;
}LineNode;

// **************************************************** function decalrations *
int loadFile(char* filePath);
void showFile();
int initList();
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
		printf(1, "Load successfully! File lines: %d\n", fileLineNum);
	}
	else {
		// something goes wrong
		printf(1, "Load unsuccessfully!\n\n");
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
	if (!initList()) {
		printf(1, "initList error!\n");
		return 0;
	}

	// the most important part
	// adding every lines into that linkedlist
	char buf[BUF_LINE_LENGTH] = {};
	int len = 0;
	int scaning = 0;
	LineNode* now = NULL;

	// read a new line
	while ((len = read(file, buf, BUF_LINE_LENGTH)) > 0) {
		int start = 0;
		int end = 0;
		// if the [start pointer] is still inline
		while (start < len) {
			// find out the line's ending
			for (end = start; end < len && buf[end] != '\n'; ++end)
				;

			// [scaning == 1] means is still reading the latest line
			if (scaning == 0) {
				now = newNode(tailNode->prev);
			}

			if (now == NULL) {
				printf(1, "nowNode pointer is NULL!\n");
				return 0;
			}

			// if is a new line
			if (now->length == 0) {
				now->content = (char*)malloc(sizeof(char)*(0 + end - start));
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
				now = NULL;
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

// display file in commandline
void showFile() {
	LineNode* now = headNode->next;
	while (now!=tailNode)
	{
		// display one line
		printf(1, "%s\n",now->content);
		now = now->next;
	}
}

// init linkedlist
int initList() {
	tailNode = (LineNode*)malloc(sizeof(LineNode));
	headNode = (LineNode*)malloc(sizeof(LineNode));
	if (tailNode == 0 || headNode == 0) {
		printf(1, "malloc error!\n");
		return 0;
	}

	tailNode->prev = headNode;
	tailNode->content = NULL;
	tailNode->length = 0;
	tailNode->next = NULL;

	headNode->prev = NULL;
	headNode->content = NULL;
	headNode->length = 0;
	headNode->next = tailNode;

	return 1;
}

// add a new node after [prev] and return it
LineNode* newNode(LineNode* prev) {
	if (prev == NULL) {
		printf(1, "the newNode's prev pointer is NULL!\n");
		return NULL;
	}
	LineNode* node;
	node = (LineNode*)malloc(sizeof(LineNode));

	if (node == NULL) {
		printf(1, "newNode alloc failed!\n");
		return NULL;
	}

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
		node->next = NULL;
	}
	node->content = NULL;
	node->length = 0;

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
