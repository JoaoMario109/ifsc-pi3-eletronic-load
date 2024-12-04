# Parser

The Parser module provides tree-based parsing functionality for data. It includes functions for parsing data from a buffer using a tree structure, matching nodes based on buffer data, and traversing through the tree.

# Usage

To use the Parser module, follow these steps:

Include the parser.h header file in your code:

``
#include "parser.h"
``

Implement the buffer consumer function. This function is responsible for consuming data from the buffer and updating the buffer pointer. It should have the following signature:

``
parser_consumer_data_t consumer(struct buffer_t *buffer);
``

The parser_consumer_data_t type is an enumeration that represents the status of the buffer consumption (OK, END_OF_BUFFER, ERROR). Make sure to define the consumer function according to your specific buffer structure and requirements.

Define match functions for comparing the buffer data with the nodes in the tree. The match function should have the following signature:

``
parser_match_t match_func(void *data, const void *match_value);
``

The match_func should compare the buffer data (data) with the value of the node (match_value) and return the result as parser_match_t (EQUAL or NOT_EQUAL). Implement the match function based on your specific matching criteria.

Create the nodes of the tree using the provided macros. Use the MAKE_NODES macro to create an array of nodes and the MAKE_NODE macro to create individual nodes. Here's an example:

``
MAKE_NODES(
    root,
    &match_func,
    {
        /* Create individual nodes */
        MAKE_NODE("Node 1", &callback_1, NULL),
        MAKE_NODE("Node 2", &callback_2, NULL),
        /* Add more nodes as needed */
    }
);
``

Each node should have a value, callback function, and a next node. The value is the data to be matched with the buffer, the callback function is called when a match is found, and the next node is the node to traverse to after the match.

Create the buffer structure and set its properties:

``
buffer_t buffer = {
    .begin = /* Pointer to the beginning of the buffer */,
    .actual = /* Pointer to the current position in the buffer */,
    .end = /* Pointer to the end of the buffer */,
    .consumer = &consumer, /* Set the consumer function */
};
``

Make sure to set the begin, actual, and end pointers to appropriate values based on your buffer structure.

Call the parser function with the buffer and root node to start the parsing process:

``
parser(&buffer, &root);
``

The parser function will parse the data in the buffer using the tree structure defined by the nodes. The parsing process will continue until the end of the buffer or an error occurs.

# Logging

The Parser module includes logging macros for printing information, warnings, and errors. By default, these macros use printf for output. You can customize the logging behavior by modifying the logging macros (LOG_INFO, LOG_WARN, LOG_ERROR) in the parser.h file.

# MAKE_NODES Macro

The MAKE_NODES macro is used to create an array of nodes representing a node group in the parsing tree. It takes the following parameters:

    NAME: Name of the array representing the node group. This name will be used to reference the node group in the code.
    MATCH_FUNC: Match function for the nodes in the group. The match function is responsible for comparing the buffer data with the node value and determining if they are equal or not.
    ...: Variadic parameter representing an array of node_t structures enclosed in curly braces {}. Each node_t structure represents an individual node in the group.

# MAKE_NODE Macro

The MAKE_NODE macro is used to create an individual parsing node. It takes the following parameters:

    VALUE: Value to be matched with the buffer. This can be a string, number, or any other data type depending on the implementation.
    CALLBACK: Callback function associated with the node. This function will be invoked when a match is found for the node during parsing.
    NEXT: Pointer to the next node in the parsing tree after the current node. This forms the hierarchical structure of the parsing tree.

# Additional Notes

    The MAKE_EMPTY_NODES and FILL_EMPTY_NODES macros are provided as a workaround to create circular nodes in the parsing tree. These macros allow you to create an empty node first, fill it with actual nodes later, and establish circular dependencies between nodes.
    The MAKE_WILDCARD_NODE macro is used to create a wildcard node that matches any value in the input buffer. Nodes after the wildcard node will be ignored during parsing.
    The parser function is responsible for parsing data from the buffer using the tree structure starting from the root node. It consumes data from the buffer and traverses the tree nodes based on the consumed data until the end of the tree or an error occurs.
    The logging macros LOG_INFO, LOG_WARN, and LOG_ERROR are defined to print log messages to the console. These can be customized or replaced with your own logging functions if desired.