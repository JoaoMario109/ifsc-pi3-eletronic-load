#ifndef PARSER_H
#define PARSER_H

#include "stdio.h"
#include "stdlib.h"

typedef enum parser_consumer_data
{
    PARSER_CONSUMER_OK,
    PARSER_CONSUMER_END_OF_BUFFER,
    PARSER_CONSUMER_ERROR,
} parser_consumer_data_t;

/**
 * Sequential buffer representation
 */
typedef struct buffer_t
{
    const void *begin;  /**< Points to the begin of the buffer */
    void *actual;       /**< Points to the to consume buffer */
    const void *end;    /**< Points to the end of the buffer */
    parser_consumer_data_t (*consumer)(struct buffer_t *buffer);   /**< Buffer consumer function */
} buffer_t;

typedef enum parser_match
{
    PARSER_MATCH_NOT_EQUAL,
    PARSER_MATCH_EQUAL,
    PARSER_MATCH_UNDEFINED,
} parser_match_t;

typedef struct node_t
{
    const void *value;   /**< Value pointer */
    void (*callback)(buffer_t *buffer, const void *value); /**< Handler function */
    const struct nodes_t *next;  /**< Next node of the tree */
} node_t;

typedef struct nodes_t
{
    const char *name;   /**< Node group name */
    const int size;     /**< size of the array of nodes */
    parser_match_t (*match_func)(void *data, const void *match_value); /**< Match function */
    const node_t *nodes;    /**< Nodes array */
} nodes_t;

/**
 * LOG prints macros, default log output is printf
 */
#ifndef LOG_INFO
#define LOG_INFO(...)     \
    printf("\033[0;32m"); \
    printf(__VA_ARGS__);  \
    printf("\033[0m\n")
#endif

#ifndef LOG_WARN
#define LOG_WARN(...)     \
    printf("\033[0;33m"); \
    printf(__VA_ARGS__);  \
    printf("\033[0m\n")
#endif

#ifndef LOG_ERROR
#define LOG_ERROR(...)    \
    printf("\033[0;31m"); \
    printf(__VA_ARGS__);  \
    printf("\033[0m\n")
#endif

/**
 * @brief Helper macro for making an array of nodes
 *
 * @param NAME Name of the array
 * @param MATCH_FUNC Match function for the nodes
 * @param ... Array of node_t
 */
#define MAKE_NODES(NAME, MATCH_FUNC, ...)             \
    const node_t NAME##_NODE[] =                      \
        __VA_ARGS__;                                  \
    const nodes_t NAME = {                            \
        .name = #NAME,                                \
        .nodes = NAME##_NODE,                         \
        .match_func = MATCH_FUNC,                     \
        .size = sizeof(NAME##_NODE) / sizeof(node_t), \
    }

/**
 * @brief Helper macro for making an empty array of nodes
 * This is a workaround to create circular nodes.
 * To create circular nodes, first create an empty node before all nodes that have dependency
 * with this node, and after those nodes use the macro FILL_EMPTY_NODES to create the nodes
 *
 * Example:
 * \code
 * MAKE_EMPTY_NODES(foo, &func);
 * MAKE_NODES(bar, &func, {MAKE_NODE("bar", NULL, &foo)});
 * FILL_EMPTY_NODES(foo, {MAKE_NODE("foo", NULL, &bar)});
 * \endcode
 *
 * In this example, the next node of foo is bar and the next node of bar is foo
 *
 * @param NAME Name of the empty node
 * @param MATCH_FUNC Match function for the node
 */
#define MAKE_EMPTY_NODES(NAME, MATCH_FUNC) \
    nodes_t NAME = {                       \
        .name = #NAME,                     \
        .match_func = MATCH_FUNC,          \
    }

/**
 * @brief Helper macro to fill an empty node
 *
 * @param NAME Name of the node
 * @param ... Array of node_t
 */
#define FILL_EMPTY_NODES(NAME, ...)         \
    const node_t NAME##_NODE[] =            \
        __VA_ARGS__;                        \
    NAME.nodes = NAME##_NODE; \
    *(int*)&NAME.size = sizeof(NAME##_NODE) / sizeof(node_t);

/**
 * @brief Helper macro for creating parsing nodes
 *
 * @param VALUE Value to be matched with the buffer
 * @param CALLBACK Callback function for the node
 * @param NEXT Next node in the tree
 */
#define MAKE_NODE(VALUE, CALLBACK, NEXT) \
    {                                    \
        .value = VALUE,                  \
        .callback = CALLBACK,            \
        .next = NEXT,                    \
    }


/**
 * @brief Helper macro for creating a wildcard parsing node.
 *
 * This macro creates a node that matches with any value in the input buffer.
 * Nodes are checked sequentially, and all nodes after this wildcard node will be ignored.
 *
 * @param CALLBACK A function pointer to the callback function associated with the wildcard node.
 * @param NEXT     A pointer to the next node in the tree after the wildcard node.
 *
 * @note The callback function will be invoked when a wildcard match is found during parsing.
 * @remark The next node in the tree should be carefully chosen to ensure correct parsing behavior.
 *         Nodes after this wildcard node will be ignored, so the wildcard node should be placed
 *         appropriately in the tree.
 */
#define MAKE_WILDCARD_NODE(CALLBACK, NEXT) \
    {                                      \
        .value = NULL,                     \
        .callback = CALLBACK,              \
        .next = NEXT,                      \
    }

/**
 * @brief Parse data from buffer using a tree structure starting from the root node
 *
 * This function parses data from the buffer using a tree structure, starting from the specified root node.
 * The buffer data is consumed using a function specified in the `buffer` parameter, and the tree nodes
 * are traversed based on the consumed data until the end of the tree is reached or an error occurs.
 *
 * @param buffer Pointer to the buffer containing the data to be parsed
 * @param root_node Pointer to the root node of the tree to be traversed
 *
 * @note This function assumes that the `buffer` argument is a valid pointer to a valid buffer_t structure,
 *       and that the `root_node` argument is a valid pointer to a valid nodes_t structure representing the root
 *       node of the tree.
 * @note The `buffer` parameter should contain a function pointer for consuming data from the buffer. The return
 *       value of the function should indicate the status of the consumption, such as OK for successful consumption
 *       and an appropriate error code for any errors that may occur.
 * @note The tree structure is expected to be properly initialized with valid nodes and match functions that
 *       correspond to the expected data format in the buffer. The caller of this function should ensure that
 *       the tree structure is correctly set up before calling this function to avoid unexpected behavior.
 * @note This function may log information, warning, or error messages using logging functions such as LOG_INFO,
 *       LOG_WARN, and LOG_ERROR. It is assumed that these logging functions are properly defined and configured
 *       in the system to provide meaningful logs for debugging and troubleshooting purposes.
 */
void parser(buffer_t *buffer, const nodes_t *root_node);

#endif