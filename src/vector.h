#ifndef __VECTOR_H
#define __VECTOR_H

/**********************************************************
 * Macros
 **********************************************************/
#define V_NARGS_IMPL_(x1, x2, x3, x4, x5, x6, x7, x8, x9, x11, x12, x13, \
        x14, x16, N, ...) N
#define V_EXPAND_(x) x
#define V_PP_NARGS_(...) \
    V_EXPAND_(V_NARGS_IMPL_(__VA_ARGS__, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, \
                5, 4, 3, 2, 1, 0))
#define V_FOREACH(_vector, _type, _name, _index)    \
    _type *_name = (_type *)vector_get(_vector, 0); \
    for (unsigned int _index = 0; _index < vector_size(_vector); _name = (_type *)vector_get(_vector, ++_index))

typedef struct vector_t vector_t;

/**********************************************************
 * \brief Init new vector
 *
 * \returns newly created vector
 **********************************************************/
vector_t* vector_new(void);


/**********************************************************
 * \brief Init new vector of size
 *
 * \param size      size of vector
 * 
 * \returns newly created vector
 **********************************************************/
vector_t* vector_new_size(int size);

/**********************************************************
 * \brief Free vector memory
 *
 * \param vector    this
 **********************************************************/
void vector_destroy(vector_t* vector);

/**********************************************************
 * \brief Get size of vector
 *
 * \param vector   this
 *
 * \returns size
 **********************************************************/
int vector_size(vector_t* vector);

/**********************************************************
 * \brief Shuffle vector randomly
 *
 * \param vector    this
 **********************************************************/
void vector_shuffle(vector_t* vector);

/**********************************************************
 * \brief Add item to vector
 *
 * \param vector   this
 * \param item     item to add to vector
 **********************************************************/
void vector_add(vector_t* vector, void* item);

/**********************************************************
 * \brief Set item in vector
 *
 * \param vector   this
 * \param item     item to set in vector
 * \param pos      position of item to set
 **********************************************************/
void vector_set(vector_t * vector, void* item, int pos);

/**********************************************************
 * \brief Add item to vector and copy data
 *
 * \param vector   this
 * \param item     item to add to vector
 **********************************************************/
void vector_pushback(vector_t* vector, void* item, int size);

/**********************************************************
 * \brief Pop last item of vector
 *
 * \param vector   this
 **********************************************************/
void vector_popback(vector_t* vector);

/**********************************************************
 * \brief Delete item from vector at givem index
 *
 * \param vector   this
 * \param index    item index to delete
 **********************************************************/
void vector_delete(vector_t* vector, int index);

/**********************************************************
 * \brief Get item from vector at given index
 *
 * \param vector   this
 * \param index    item to add to vector
 *
 * \returns item if found NULL if not
 **********************************************************/
void* vector_get(vector_t* vector, int index);

#endif /* __VECTOR_H */
