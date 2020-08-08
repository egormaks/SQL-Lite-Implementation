# SQL-Lite-Implementation
This project involves writing a clone of SQL lite from scratch in C.
SQL-Lite follows the following logical pathway to execute a query:  
  Tokenizer -> Parser -> Code Generator -> Virtual Machine -> B-Tree -> Pager -> OS Interface
We will implement all stages of a query.

The tokenizer -> parser -> code generator chain is the "front end". It takes a SQL query and outputs virtual machine byte-code. 

The virtual machine -> b-tree -> pager -> OS interface is the back-end. 
	Virtual Machine:
		Takes bytecode and then performs operations on one or more tables, which are stored in a B-Tree. 
	B-Trees:
		Consist of many nodes, each node being one page length. The B-tree retrieves a page from disk or save it back to disk via pager.
	Pager:
		Receives commands to read/write pages of data. 
	OS-interface:
		Differs based on the OS that sql lite is compiled for. 
