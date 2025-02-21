/** @file hashtable.c
 *
 * @brief Hashtable data structure used to store key-value pairs.
 */

#include "../include/hashtable.h"

/*! 
 * @brief The hash function used to index the keys for the hashtable (DJB2).
 * @param[in] key A pointer to the key abstract data.
 * @param[in] key_type Specifies whether the key is a string or an integer.
 * @return Returns an index for the hashtable.
 */
static unsigned long hashtable_hash(void * key, char key_type)
{
    unsigned long hash = 5381;
    int c;

    if (key_type == 's')
    {
        unsigned char *str = (unsigned char *)key;
        while ((c = *str++))
        {
            hash = ((hash << 5) + hash) + c;
        }
    }
    // Handles case where key is an integer.
    else if (key_type == 'd')
    {
        // Dereference the integer pointer to get the value.
        int * int_key = (int *)key;
        hash = *int_key;
    }

    return hash % TABLE_SIZE;
} /* hashtable_hash() */

/*!
 * @brief Initializes the item data structure to be injected into the hashtable.
 * @param[in] key A pointer to the key abstract data.
 * @param[in] value A pointer to the value abstract data.
 * @param[in] key_type Specifies whether the key is a string or an integer.
 * @return Returns a pointer to the item.
 */
static item_t * hashtable_create_item(void * key, void * value, char key_type)
{
    item_t * new_item = (item_t *)malloc(sizeof(item_t));
    if (!new_item)
    {
        printf("Memory allocation failed\n");
        goto exit;
    }

    if (key_type == 's')
    {
        new_item->key = strndup((char *)key, MAX_SIZE);
    }
    else if (key_type == 'd')
    {
        new_item->key = malloc(sizeof(int));
        if (!new_item->key)
        {
            printf("Memory allocation failed\n");
            free(new_item);
            new_item = NULL;
            goto exit;
        }
        *(int *)new_item->key = *(int *)key;
    }

    new_item->value = value;
    new_item->next = NULL;
    exit:
    return new_item;
} /* hashtable_create_item() */

/*!
 * @brief Initializes the hashtable datastructure.
 * @return Returns a pointer to the allocated hashtable.
 */
hashtable_t * hashtable_init(void)
{
    hashtable_t * new_table = (hashtable_t *)malloc(sizeof(hashtable_t));
    if (!new_table)
    {
        printf("Memory allocation failed\n");
        goto exit;
    }

    new_table->table = (item_t **)calloc(TABLE_SIZE, sizeof(item_t *));
    if (!new_table->table)
    {
        printf("Memory allocation failed\n");
        free(new_table);
        new_table = NULL;
        goto exit;
    }

    exit:
    return new_table;
} /* hashtable_init() */

/*!
 * @brief Inserts the key-value pair into the hashtable based on the key type.
 * @param[in] hashtable A pointer to the hashtable data structure.
 * @param[in] key A pointer to the key abstract data.
 * @param[in] value A pointer to the value abstract data.
 * @param[in] key_type Specifies whether the key is a string or an integer.
 */
void hashtable_insert(hashtable_t * hashtable, void * key, void * value, char key_type)
{
    unsigned int index = hashtable_hash(key, key_type);
    item_t * new_item = hashtable_create_item(key, value, key_type);

    // Check if the index is empty or if a collision occurs.
    if (hashtable->table[index] == NULL)
    {
        hashtable->table[index] = new_item;
    }
    else
    {
        item_t * item = hashtable->table[index];
        while (item)
        {
            // Update the item if the key exists.
            if ((key_type == 's' && strcmp(item->key, key) == 0) ||
                (key_type == 'd' && *(int *)item->key == *(int *)key))
            {
                item->value = value;
                free(new_item->key);
                free(new_item);
                return;
            }
            item = item->next;
        }

        // No match, add new item to the list.
        new_item->next = hashtable->table[index];
        hashtable->table[index] = new_item;
    }
} /* hashtable_insert() */

/*!
 * @brief Searches the hashtable to see if a value exists for the key specified.
 * @param[in] hashtable A pointer to the hashtable data structure.
 * @param[in] key A pointer to the key abstract data.
 * @param[in] key_type Specifies whether the key is a string or an integer.
 * @return Returns a pointer to the data if it exists in the hashtable, otherwise NULL.
 */
void * hashtable_search(hashtable_t * hashtable, void * key, char key_type)
{
    unsigned int index = hashtable_hash(key, key_type);
    item_t * item = hashtable->table[index];

    while (item)
    {
        // Compare the keys for string or integer types.
        if ((key_type == 's' && strcmp(item->key, key) == 0) ||
            (key_type == 'd' && *(int *)item->key == *(int *)key))
        {
            return item->value;
        }
        item = item->next;
    }

    return NULL;
} /* hashtable_search() */

/*!
 * @brief Removes the key-value pair if the key exists in the hashtable.
 * @param[in] hashtable A pointer to the hashtable data structure.
 * @param[in] key A pointer to the key abstract data.
 * @param[in] key_type Specifies whether the key is a string or an integer.
 */
void hashtable_remove(hashtable_t * hashtable, void * key, char key_type)
{
    unsigned int index = hashtable_hash(key, key_type);
    item_t * item = hashtable->table[index];
    item_t * prev_item = NULL;

    while (item)
    {
        if ((key_type == 's' && strcmp(item->key, key) == 0) ||
            (key_type == 'd' && *(int *)item->key == *(int *)key))
        {
            if (prev_item)
            {
                prev_item->next = item->next;
            }
            else
            {
                hashtable->table[index] = item->next;
            }
            free(item->key);
            free(item);
            return;
        }
        prev_item = item;
        item = item->next;
    }

    printf("Key not found\n");
} /* hashtable_remove() */

/*!
 * @brief Frees any data that was allocated for the hashtable.
 * @param[in] hashtable A pointer to the hashtable data structure.
 */
void hashtable_free(hashtable_t * hashtable)
{
    for (int i = 0; i < TABLE_SIZE; i++)
    {
        item_t * item = hashtable->table[i];
        while (item)
        {
            item_t * temp = item;
            item = item->next;
            free(temp->key);
            free(temp);
        }
    }

    free(hashtable->table);
    free(hashtable);
} /* hashtable_free() */

/*!
 * @brief Frees any data specifically values that was allocated for the hashtable.
 * @param[in] hashtable A pointer to the hashtable data structure.
 */
void hashtable_free_values(hashtable_t * hashtable)
{
    for (int i = 0; i < TABLE_SIZE; i++)
    {
        item_t * item = hashtable->table[i];
        while (item)
        {
            item_t * temp = item;
            item = item->next;
            free(temp->value);
        }
    }
} /* hashtable_free_value() */

/*** end of file ***/