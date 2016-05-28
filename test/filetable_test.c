#include "../common/filetable.h"


int main(){

	FileTable* table = initTable("./tmp");
	printTable(table);

	deleteNode(table, "dummy3.txt");
	deleteNode(table, "dummy1.txt");
	printTable(table);
	deleteNode(table, "dummy2.txt");
	printTable(table);
	destroyTable(table);
}
