/**
 * @file parser.c
 *
 * @brief Implementation of the parser module for tree-based parsing
 *
 * This file contains the implementation of the parser module, which is used for tree-based parsing of data.
 * It includes functions for parsing one level of the tree, handling buffer data and node matching, and
 * traversing through the tree structure. It also includes logging functions for debugging and troubleshooting
 * purposes.
*/


#include "parser.h"

/**
 * @brief Parse one level of the tree receiving the buffer and the node that it will match
 *
 * This function parses one level of a tree structure by matching the data in the buffer with the nodes
 * specified by the `nodes` argument. The `buffer` parameter contains the data to be parsed, and the `nodes`
 * parameter specifies the nodes that the buffer data will be matched against.
 *
 * @param buffer Pointer to the buffer containing the data to be parsed
 * @param nodes Pointer to the nodes that the buffer data will be matched against
 *
 * @return const nodes_t* Pointer to the next level of the tree after parsing the buffer data
 *
 * @note This function assumes that the `nodes` argument is a valid pointer to a valid nodes_t structure,
 *       and that the `buffer` argument is a valid pointer to a valid buffer_t structure.
 * @note This function may return NULL if an error occurs or if the actual node is a leaf node.
 * @note The caller of this function should handle the NULL node that represent an end of tree or an error
 * @note This function may log information, warning, or error messages using logging functions such as LOG_INFO,
 *       LOG_WARN, and LOG_ERROR. It is assumed that these logging functions are properly defined and configured
 *       in the system to provide meaningful logs for debugging and troubleshooting purposes.
 */
const nodes_t *node_parser(buffer_t *buffer, const nodes_t *nodes);

const nodes_t *node_parser(buffer_t *buffer, const nodes_t *nodes)
{
    const node_t *node = nodes->nodes;
    LOG_INFO("parsing node %s", nodes->name);
    /* Check for a match in the level of the tree */
    for (int i = 0; i < nodes->size; i++)
    {
        if (nodes->match_func == NULL)
        {
            LOG_ERROR("inexistent match function");
            return NULL;
        }

        /* check if buffer data match with the node or if is a wildcard node*/
        if (node[i].value == NULL || nodes->match_func(buffer->actual, node[i].value) == PARSER_MATCH_EQUAL)
        {
            if (node[i].callback != NULL)
                node[i].callback(buffer, node[i].value);

            //LOG_INFO("next node is %s", node[i].next != NULL ? node[i].next->name : "NULL");
            return node[i].next;
        }
    }
    LOG_WARN("Unrecognized command\n");
    return NULL;
}

void parser(buffer_t *buffer, const nodes_t *root_node)
{
    const nodes_t *nodes = root_node;
    for (parser_consumer_data_t result = PARSER_CONSUMER_OK; result == PARSER_CONSUMER_OK; result = buffer->consumer(buffer))
    {
        if (nodes == NULL)
        {
            //LOG_INFO("end of tree");
            return;
        }
        nodes = node_parser(buffer, nodes);
    }
}