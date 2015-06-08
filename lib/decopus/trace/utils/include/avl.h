#include <stdlib.h>
#include <stdbool.h>

#ifdef AVL_DEBUG
#include <assert.h>
#define AVL_PRINT
#endif
#ifdef AVL_PRINT
#include <stdio.h>
#endif
#if defined(AVL_DEBUG) && AVL_DEBUG > 0
#define AVL_debug(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)
#else
#define AVL_debug(fmt, ...)
#endif

/****************************
 * AVL Balanced binary tree *
 ****************************
 *
 * Main parameters (have to be defined before):
 * + AVL_prefix -> Prefix of every type/function names in this file.
 *                 in this file every name is 'AVL_somename', 'AVL_' will
 *                 be replaced (by the preprocessor) by AVL_prefix.
 *                 (example: with "#define AVL_prefix myavl", the node type
 *                 will be myavl_node_t)
 * + AVL_key_t  -> Type of the key used to sort the nodes in the tree.
 * + AVL_data_t -> Type of the additional data stored in a node.
 *                 (optional: no data if undefined)
 *
 * Main types:
 * + AVL_tree_t -> The tree type (ie: the root)
 * + AVL_node_t -> The node containing the key and data
 * + AVL_pool_t -> Pool of nodes
 *
 * Main functions:
 * + void AVL_init(AVL_tree_t *tree, AVL_pool_t *pool)
 *              -> tree != NULL
 *              -> initialize the 'tree' structure
 *                 if 'pool' != NULL, it will be used to get/release nodes
 * + void AVL_reset(AVL_tree_t *tree)
 *              -> tree != NULL
 *              -> remove all nodes from the tree
 * + bool AVL_find(AVL_tree_t *tree, AVL_key_t key, AVL_node_t **node)
 *              -> tree != NULL, node != NULL
 *              -> Find a node with the given 'key'.
 *              -> Returns true if one is found.
 *              -> *node is either set to the found node or to the 
 *              -> previous/next node in key order.
 * + bool AVL_add(AVL_tree_t *tree, AVL_key_t key, AVL_node_t **node)
 *              -> tree != NULL, node != NULL
 *              -> Add (if the key is not already in the tree) a new node.
 *              -> *node have to be either NULL, or an ancester of the
 *              -> existing(/previous/next if not already existing) node.
 *              -> Note: using the result of find with the same key is valid.
 *              -> Returns true if a node is added.
 *              -> *node is set to the existing/new node.
 * + void AVL_rem(AVL_tree_t *tree, AVL_node_t* node)
 *              -> tree != NULL, node != NULL
 *              -> Remove the 'node' from the 'tree'.
 * + AVL_node_t *AVL_first(AVL_tree_t *tree)
 * + AVL_node_t *AVL_last(AVL_tree_t *tree)
 *              -> tree != NULL
 *              -> Returns the first/last node in key order.
 *              -> Returns NULL if the tree is empty.
 * + AVL_node_t *AVL_prev(AVL_tree_t *tree, AVl_node_t *node)
 * + AVL_node_t *AVL_next(AVL_tree_t *tree, AVl_node_t *node)
 *              -> tree != NULL, node != NULL
 *              -> Returns the prev/next node of 'node' in key order.
 *              -> Returns NULL if there is no such node.
 *
 * Pool parameters:
 * + AVL_POOL_ARRAY_SIZE -> Number of nodes allocated when we need one.
 *                          Default is 100.
 *
 * Pool functions:
 * + void AVL_pool_init (AVL_pool_t *pool)
 *              -> pool != NULL
 *              -> Initialize a pool structure.
 * + void AVL_pool_term (AVL_pool_t *pool)
 *              -> pool != NULL
 *              -> Free all elements allocated in this pool.
 *
 * Additional parameters:
 * + AVL_LINKED -> If defined, the nodes are linked in key and reverse order,
 *                 allowing to fasten the walk through the nodes.
 */

#ifndef AVL_prefix
#error "AVL_prefix is undefined !"
#endif
#ifndef AVL_key_t
#error "AVL_key_t is undefined !"
#endif

#define AVL_CONCAT(x,y) x ## _ ## y
#define AVL_XCONCAT(x,y) AVL_CONCAT(x,y)

#define AVL_tree_t  AVL_XCONCAT(AVL_prefix, tree_t)
#define AVL_node_t  AVL_XCONCAT(AVL_prefix, node_t)
#define AVL_pool_t  AVL_XCONCAT(AVL_prefix, pool_t)
#define AVL_array_t AVL_XCONCAT(AVL_prefix, array_t)

#define AVL_pool_create AVL_XCONCAT(AVL_prefix, pool_create)
#define AVL_pool_delete AVL_XCONCAT(AVL_prefix, pool_delete)
#define AVL_pool_init   AVL_XCONCAT(AVL_prefix, pool_init)
#define AVL_pool_term   AVL_XCONCAT(AVL_prefix, pool_term)

#define AVL_node_create AVL_XCONCAT( , AVL_XCONCAT(AVL_prefix, node_create))
#define AVL_node_delete AVL_XCONCAT( , AVL_XCONCAT(AVL_prefix, node_delete))
#define AVL_balance     AVL_XCONCAT( , AVL_XCONCAT(AVL_prefix, balance))
#define AVL_rot         AVL_XCONCAT( , AVL_XCONCAT(AVL_prefix, rot))

#define AVL_print         AVL_XCONCAT(AVL_prefix, print)
#define AVL_print_r       AVL_XCONCAT(AVL_prefix, print_r)
#define AVL_check_tree    AVL_XCONCAT(AVL_prefix, check_tree)
#define AVL_check_subtree AVL_XCONCAT(AVL_prefix, check_subtree)

#define AVL_init        AVL_XCONCAT(AVL_prefix, init)
#define AVL_reset       AVL_XCONCAT(AVL_prefix, reset)
#define AVL_first       AVL_XCONCAT(AVL_prefix, first)
#define AVL_last        AVL_XCONCAT(AVL_prefix, last)
#define AVL_prev        AVL_XCONCAT(AVL_prefix, prev)
#define AVL_next        AVL_XCONCAT(AVL_prefix, next)
#define AVL_find        AVL_XCONCAT(AVL_prefix, find)
#define AVL_add         AVL_XCONCAT(AVL_prefix, add)
#define AVL_rem         AVL_XCONCAT(AVL_prefix, rem)
#define AVL_add_ranged  AVL_XCONCAT(AVL_prefix, add_ranged)

#ifndef AVL_PRINT_OUTPUT
#ifdef AVL_DEBUG
#define AVL_PRINT_OUTPUT stderr
#else
#define AVL_PRINT_OUTPUT stdout
#endif
#endif

#define _AVL_FUNC_ATTRIB static __attribute((__unused__))

#ifndef AVL_SIDE_T
#define AVL_SIDE_T
enum avl_side_t {
   AVL_LEFT = 0,
   AVL_RIGHT = 1
};
#endif

typedef struct AVL_node_t AVL_node_t;
struct AVL_node_t {
   AVL_node_t     *father;
   AVL_node_t     *child[2]; // 0->child[AVL_LEFT] node 1->child[AVL_RIGHT] node
   signed char     balance;  // balance of this subtree
   enum avl_side_t side:8;     // side of this node in father node
#ifdef AVL_LINKED
   AVL_node_t     *neighbor[2];
#endif
   AVL_key_t       key;
#ifdef AVL_RANGED
   AVL_key_t       max;
#endif
#ifdef AVL_data_t
   AVL_data_t      data;
#endif
};

#ifndef AVL_POOL_ARRAY_SIZE
#define AVL_POOL_ARRAY_SIZE 100
#endif
typedef struct AVL_array_t AVL_array_t;
struct AVL_array_t {
   AVL_array_t *next;
   AVL_node_t   nodes[AVL_POOL_ARRAY_SIZE];
};
typedef struct AVL_pool_t {
   AVL_node_t  *list; /* linked list (using father field) of free node */
   AVL_array_t *array; /* linked list of allocated array */
   unsigned int nbnode;
} AVL_pool_t;


typedef struct AVL_tree_t {
   AVL_node_t *root;
#ifdef AVL_LINKED
   AVL_node_t     *first;
   AVL_node_t     *last;
#endif
#ifndef AVL_DYNPOOL
   AVL_pool_t *pool;
#endif
#ifdef AVL_STAT
   unsigned int nbnode;
#endif
} AVL_tree_t ;

#ifdef AVL_PRINT
/*
 * print a subtree to the given depth
 */
static int AVL_print_r (const AVL_node_t *node, unsigned int depth, 
      const AVL_node_t *father, enum avl_side_t side, int cur)
{
   int res = 0;
   if (node && depth > 0)
      res += AVL_print_r(node->child[AVL_LEFT], depth - 1, node, AVL_LEFT, cur + 1);
   
   {
      char buffer[2*cur+2];
      char bufr[2*cur+2];
      char bufl[2*cur+2];
      for (int i = 0; i < 2*cur; i += 1) {
         buffer[i] = ' ';
         bufr[i] = ' ';
         bufl[i] = ' ';
      }
      buffer[2*cur] = cur?'+':'$';
      buffer[2*cur+1] = '\0';
      bufr[2*cur] = '\n';
      bufr[2*cur+1] = '\0';
      bufl[2*cur] = '\n';
      bufl[2*cur+1] = '\0';
      if (cur > 0) {
         buffer[2*cur - 2] = '+';
         buffer[2*cur - 1] = '-';
         if (side == AVL_LEFT)
            bufr[2*cur - 2] = '/';
         else
            bufl[2*cur - 2] = '\\';
      }
      for (int i = cur - 2; i >= 0; i -= 1) {
         if (father->side != side) {
            buffer[2*i] = (father->side==AVL_LEFT)?'/':'\\';
            bufr[2*i]   = (father->side==AVL_LEFT)?'/':'\\';
            bufl[2*i]   = (father->side==AVL_LEFT)?'/':'\\';
         }
         side = father->side;
         father = father->father;
      }
      if (node && depth) {
         res += 1;
         bufr[0] = '\0';
         bufl[0] = '\0';
      }
      fprintf(AVL_PRINT_OUTPUT, "%s%s (", bufl, buffer);
      if (node) {
#ifdef AVL_RANGED
         fprintf(AVL_PRINT_OUTPUT, "%p[0x%llx->0x%llx]", 
               node, (long long) node->key, (long long) node->max);
#else
         fprintf(AVL_PRINT_OUTPUT, "%p[0x%llx]", node, (long long) node->key);
#endif
#if defined(AVL_PRINT_DATA) && defined(AVL_data_t)
         {
            char str[11];
            AVL_PRINT_DATA(&node->data, str);
            str[10] = '\0';
            fprintf(AVL_PRINT_OUTPUT, ", Data=%s", str);
         }
#endif
         fprintf(AVL_PRINT_OUTPUT, ", Bal=%d)", node->balance);
      } else
         fprintf(AVL_PRINT_OUTPUT, "null)");
      fprintf(AVL_PRINT_OUTPUT, "\n%s", bufr);
   }

   if (node && depth > 0)
         res += AVL_print_r(node->child[AVL_RIGHT], depth - 1, node, AVL_RIGHT, cur + 1);

   return res;
}

#ifdef AVL_DEBUG   
static int AVL_check_subtree (const AVL_node_t *node, bool print);
#endif
static __attribute__((__unused__)) 
void AVL_print (const AVL_node_t *node, int depth)
{
   int cnt = AVL_print_r(node, depth, NULL, 0, 0);
   fprintf(AVL_PRINT_OUTPUT,"%d nodes diplayed\n", cnt);
   fflush(AVL_PRINT_OUTPUT);
#ifdef AVL_DEBUG
   AVL_check_subtree(node, false);
#endif
   
}
#endif /* AVL_PRINT */

/*
 * check if a tree/subtree is correct
 */
#ifdef AVL_DEBUG
#ifdef AVL_RANGED
#define AVL_CHECK(cond) \
   do { \
      if (!(cond)) { \
         ok = false; \
         fprintf(stderr, "failed{node %p [0x%llx;0x%llx]}: (line %d)" #cond "\n", \
               node, \
               (unsigned long long) (node?node->key:0), \
               (unsigned long long) (node?node->max:0), \
               __LINE__); \
      } \
   } while (0)
#else
#define AVL_CHECK(cond) \
   do { \
      if (!(cond)) { \
         ok = false; \
         fprintf(stderr, "failed{node %p [0x%llx]}: (line %d)" #cond "\n", \
               node, \
               (unsigned long long) (node?:node->key:0), \
               __LINE__); \
      } \
   } while (0)
#endif
static int AVL_check_subtree (const AVL_node_t *node, bool print)
{
   bool ok = true;
   if (node == NULL)
      return 0;
#ifdef AVL_RANGED
   AVL_CHECK(node->key <= node->max);
#endif
   if (node->father) {
      AVL_CHECK(node->father->child[node->side] == node);
   }

   if (node->child[AVL_LEFT]) {
      AVL_CHECK(node->child[AVL_LEFT]->side == AVL_LEFT);
#ifdef AVL_RANGED
      AVL_CHECK(node->child[AVL_LEFT]->max < node->key);
#else
      AVL_CHECK(node->child[AVL_LEFT]->key < node->key);
#endif
   }
   if (node->child[AVL_RIGHT]) {
      AVL_CHECK(node->child[AVL_RIGHT]->side == AVL_RIGHT);
#ifdef AVL_RANGED
      AVL_CHECK(node->child[AVL_RIGHT]->key > node->max);
#else
      AVL_CHECK(node->child[AVL_RIGHT]->key > node->key);
#endif
   }

   int r = AVL_check_subtree(node->child[AVL_RIGHT], print);
   int l = AVL_check_subtree(node->child[AVL_LEFT], print);
   if (r < 0 || l < 0)
      return -1;

   AVL_CHECK(node->balance == (r - l));
   AVL_CHECK(node->balance * node->balance <= 1);

   if (!ok) {
      if (print)
         AVL_print(node, 10);
      return -1;
   }
   return 1 + ((r>l)?r:l);
}
int AVL_check_tree (const AVL_tree_t *tree)
{
   bool ok = true;
   if (tree->root) {
      AVL_node_t *node = tree->root;
      AVL_CHECK(tree->root->father == NULL);
   }
   
   if(AVL_check_subtree(tree->root, true) < 0 || !ok)
      return -1;

#ifdef AVL_LINKED
   if (tree->root) {
      AVL_node_t *node = tree->root;
      while (node->child[AVL_LEFT])
         node = node->child[AVL_LEFT];
      AVL_CHECK(node->neighbor[AVL_LEFT] == NULL);
      do {
         if (node->neighbor[AVL_RIGHT] == NULL)
            break;
         AVL_CHECK(node->neighbor[AVL_RIGHT]->neighbor[AVL_LEFT] == node);
#ifdef AVL_RANGED
         AVL_CHECK(node->max < node->neighbor[AVL_RIGHT]->key);
#else
         AVL_CHECK(node->key < node->neighbor[AVL_RIGHT]->key);
#endif
         node = node->neighbor[AVL_RIGHT];
      } while (1);
      AVL_CHECK(node->child[AVL_RIGHT] == NULL);
   }
#endif

   if (!ok) {
      AVL_print(tree->root, 16);
      return -1;
   }
   
   return 0;
}
#undef AVL_CHECK
#endif


/*
 * init a pool
 */
_AVL_FUNC_ATTRIB void AVL_pool_init (AVL_pool_t *pool)
{
   pool->list = NULL;
   pool->array = NULL;
   pool->nbnode = 0;
   return;
}

/*
 * create a pool
 */
_AVL_FUNC_ATTRIB AVL_pool_t * AVL_pool_create ()
{
   AVL_pool_t *pool = malloc(sizeof(AVL_pool_t));
   AVL_pool_init(pool);
   return pool;
}

/*
 * terminate a pool
 */
_AVL_FUNC_ATTRIB void AVL_pool_term (AVL_pool_t *pool)
{
   AVL_array_t *cur = pool->array;
   while (cur != NULL) {
      AVL_array_t *tmp = cur;
      cur = cur->next;
      free(tmp);
   }
}

/*
 * delete a pool
 */
_AVL_FUNC_ATTRIB void AVL_pool_delete (AVL_pool_t *pool)
{
   AVL_pool_term(pool);
   free(pool);
}

/*
 * create a new node (internal function)
 */
static AVL_node_t * AVL_node_create (
      AVL_tree_t *tree
#ifdef AVL_DYNPOOL
      , AVL_pool_t *pool
#endif
      )
{
#ifdef AVL_STAT
   tree->nbnode += 1;
#endif

#ifdef AVL_SAFEMODE
   return calloc(1, sizeof(AVL_node_t));
#else

#ifndef AVL_DYNPOOL
   AVL_pool_t *pool = tree->pool;
#endif

   if (pool == NULL)
      return malloc(sizeof(AVL_node_t));

   if (pool->list == NULL) {
      // adding a node array to the pool
      AVL_array_t *array = malloc(sizeof(AVL_array_t));
      array->next = pool->array;
      pool->array = array; 
      for (int i = 0; i < AVL_POOL_ARRAY_SIZE - 1; i += 1) {
         array->nodes[i].father = &array->nodes[i+1];
      }
      array->nodes[AVL_POOL_ARRAY_SIZE - 1].father = NULL;
      pool->list = &array->nodes[0];
      pool->nbnode += AVL_POOL_ARRAY_SIZE;
   }

   AVL_node_t *node = pool->list;
   pool->list = node->father;
   return node;
#endif
}

/*
 * delete a node (internal function)
 */
static void AVL_node_delete (
      AVL_tree_t *tree,
#ifdef AVL_DYNPOOL
      AVL_pool_t *pool,
#endif
      AVL_node_t *node)
{
#ifdef AVL_STAT
   tree->nbnode -= 1;
#endif
  
#ifdef AVL_SAFEMODE
   free(node);
#else 

#ifndef AVL_DYNPOOL
   AVL_pool_t *pool = tree->pool;
#endif
   if (pool == NULL) {
      free(node);
      return;
   }
   
   node->father = pool->list;
   pool->list = node;
   return;
#endif
}

/*
 * init a tree
 */
_AVL_FUNC_ATTRIB void AVL_init (
      AVL_tree_t *tree, 
#ifndef AVL_DYNPOOL
      AVL_pool_t *pool
#endif
      )
{
   tree->root = NULL;
#ifdef AVL_LINKED
   tree->last = NULL;
   tree->first = NULL;
#endif

#ifndef AVL_DYNPOOL
   tree->pool = pool;
#endif

#ifdef AVL_STAT
   tree->nbnode = 0;
#endif
}

/*
 * reset a tree
 */
#define AVL_reset_callback(name) void(*name)(AVL_node_t *)
_AVL_FUNC_ATTRIB void AVL_reset (
      AVL_tree_t *tree,
#ifdef AVL_DYNPOOL
      AVL_pool_t *pool,
#endif
      AVL_reset_callback(cb))
{

   AVL_node_t *todo = tree->root;
   while (todo) {
      AVL_node_t *cur = todo;
      todo = cur->father;
      if (cur->child[AVL_LEFT]) {
         cur->child[AVL_LEFT]->father = todo;
         todo = cur->child[AVL_LEFT];
      }
      if (cur->child[AVL_RIGHT]) {
         cur->child[AVL_RIGHT]->father = todo;
         todo = cur->child[AVL_RIGHT];
      }
      if (cb != NULL)
         cb(cur);

#ifdef AVL_DYNPOOL
      AVL_node_delete(tree, pool, cur);
#else
      AVL_node_delete(tree, cur);
#endif

   }
   tree->root = NULL;
#ifdef AVL_LINKED
   tree->last = NULL;
   tree->first = NULL;
#endif
}
#undef AVL_reset_callback

/*
 * find first node
 */
_AVL_FUNC_ATTRIB AVL_node_t * AVL_first (AVL_tree_t *tree)
{
#ifdef AVL_LINKED
   return tree->first;
#else
   AVL_node_t *node = tree->root;
   AVL_node_t *prev = NULL;
   while (node) {
      prev = node;
      node = node->child[AVL_LEFT];
   }
   return prev;
#endif
}

/*
 * find last node
 */
_AVL_FUNC_ATTRIB AVL_node_t * AVL_last (AVL_tree_t *tree)
{
#ifdef AVL_LINKED
   return tree->last;
#else
   AVL_node_t *node = tree->root;
   AVL_node_t *prev = NULL;

   while (node) {
      prev = node;
      node = node->child[AVL_RIGHT];
   }
   return prev;
#endif
}

/*
 * find a node
 * *node is set to the found node 
 * (or to a previous/next node in the tree when the search is not sucessful)
 */
_AVL_FUNC_ATTRIB bool AVL_find (AVL_tree_t *tree, AVL_key_t key, AVL_node_t **node)
{
   *node = NULL;
   AVL_node_t *cur = tree->root;
   while (cur != NULL) {
      *node = cur;
      if (key < cur->key)
         cur = cur->child[AVL_LEFT];
#ifdef AVL_RANGED
      else if (key > cur->max)
#else
      else if (key > cur->key)
#endif
         cur = cur->child[AVL_RIGHT];
      else
         return true;
   }

   return false;
}

/*
 * find prev node
 */
_AVL_FUNC_ATTRIB AVL_node_t * AVL_prev (AVL_tree_t *tree __attribute__((__unused__)), AVL_node_t *node)
{
#ifdef DEBUG
   assert(node != NULL);
#endif
#ifdef AVL_LINKED
   return node->neighbor[AVL_LEFT];
#else
   if (node->child[AVL_LEFT]) {
      node = node->child[AVL_LEFT];
      while (node->child[AVL_RIGHT])
         node = node->child[AVL_RIGHT];
      return node;
   }
   enum avl_side_t side = node->side;
   node = node->father;
   while (node && side == AVL_LEFT) {
      side = node->side;
      node = node->father;
   }
   return node;
#endif
}

/*
 * find next node
 */
_AVL_FUNC_ATTRIB AVL_node_t * AVL_next (AVL_tree_t *tree __attribute__((__unused__)), AVL_node_t *node)
{
#ifdef DEBUG
   assert(node != NULL);
#endif
#ifdef AVL_LINKED
   return node->neighbor[AVL_RIGHT];
#else
   if (node->child[AVL_RIGHT]) {
      node = node->child[AVL_RIGHT];
      while (node->child[AVL_LEFT])
         node = node->child[AVL_LEFT];
      return node;
   }
   enum avl_side_t side = node->side;
   node = node->father;
   while (node && side == AVL_RIGHT) {
      side = node->side;
      node = node->father;
   }
   return node;
#endif
}

/*
 * rotate the root of the given subtree
 * return the new subtree root
 *
 * *** side == AVL_RIGHT
 * 
 * the root must have a left child
 *
 *         Root             L
 *         / \             / \
 *        /   \           /   \
 *       L     R   ==>   LL    Root
 *      / \                   / \
 *     LL  LR                LR  R
 *
 *  *** side == AVL_LEFT
 * the root must have a right child
 *
 *         Root             R
 *         / \             / \
 *        /   \           /   \
 *       L     R   ==>  Root   RR
 *            / \       / \
 *           RL  RR    L   RL
 */
static AVL_node_t * AVL_rot(AVL_tree_t *tree, AVL_node_t *root, enum avl_side_t side)
{
   AVL_debug("    AVL_rot(%p, %p, %d)\n", tree, root, side);
   
   const enum avl_side_t invside = 1 - side;
   // node is the new root
   
   AVL_node_t *node = root->child[invside];
   
   
   // set up father link of node
   node->father = root->father;
   node->side = root->side;
   if (node->father != NULL)
      node->father->child[node->side] = node;
   else
      tree->root = node;
  
   // move node->child[side] to root->child[invside]
   root->child[invside] = node->child[side];
   if (root->child[invside]) {
      root->child[invside]->father = root;
      root->child[invside]->side = invside;
   }

   // set up link between node and root
   node->child[side] = root;
   root->father = node;
   root->side = side;

   return node;
}

/*
 * balance the tree
 * add tell if we'are adding or removing a node
 * childs of node have to be well balanced
 * AVL_balance must be called on the father of child add/del
 */
static void AVL_balance(AVL_tree_t *tree, AVL_node_t *node, bool add) 
{
   AVL_debug("  AVL_balance(%p, %p(0x%llx), %s)\n", tree, node, (unsigned long long) (node?node->key:0), add?"add":"rem");

   while (node) {
      AVL_debug("  -> balance(%p(0x%llx))\n", node, (unsigned long long) node->key);
      //AVL_print(node, 4);

      /*
       * diff will contains the variation of node depth
       * 0 means we can stop
       */
      int diff = 0;

      /*
       * the following will balance 'node' subtree (if needed) and 
       * compute its depth variation
       */

      switch (node->balance) {
         case 0:
            /*
             * node was unbalanced and is now balanced.
             *
             * addition:
             *    the least deep child increased
             *
             * deletion:
             *    the deepest child decreased
             *    node's depth decrease
             */
            if (!add)
               diff = -1;
            break;
         
         case 1:
         case -1:
            /*
             *
             * addition:
             *    * depth increases (node was balanced before 'add')
             *
             * deletion:
             *    * depth remains unchanged (node was balanced before 'rm')
             */
            if (add)
               diff = 1;
            break;

         case -2:
            switch (node->child[AVL_LEFT]->balance) {
               case -1:
                  /*
                   *       N(n)(-2)              L(n-1)(0)
                   *         / \                    / \
                   *        /   \                  /   \
                   *       /     \                /     \
                   * L(n-1)(-1)  R(n-3)  ==>  LL(n-2)  N(n-2)(0)
                   *     / \                            / \
                   *    /   \                          /   \
                   *   /     \                        /     \
                   * LL(n-2)  LR(n-3)            LR(n-3)    R(n-3)
                   *
                   * addition:
                   *    L increased: subtree depth remains unchanged (n-1)
                   * deletion:
                   *    R decreased: subtree depth decreases from n to n-1
                   */
                  node = AVL_rot(tree, node, AVL_RIGHT);
                  node->balance = 0;
                  node->child[AVL_RIGHT]->balance = 0;
                  if (!add)
                     diff = -1;
                  break;
               case 0:
                  /*
                   *       N(n)(-2)               L(n)(1)
                   *         / \                    / \
                   *        /   \                  /   \
                   *       /     \                /     \
                   * L(n-1)(0)   R(n-3)  ==>  LL(n-2)  N(n-1)(-1)
                   *     / \                            / \
                   *    /   \                          /   \
                   *   /     \                        /     \
                   * LL(n-2)  LR(n-2)            LR(n-2)    R(n-3)
                   *
                   * addition:
                   *    L increased: subtree depth increases from n-1 to n
                   * deletion:
                   *    R decreased: subtree depth remains unchanged (n)
                   */
                  node = AVL_rot(tree, node, AVL_RIGHT);
                  node->balance = 1;
                  node->child[AVL_RIGHT]->balance = -1;
                  if (add)
                     diff = 1;
                  break;
               case 1:
                  /*
                   *     N(n)(-2)                N                     LR(n-1)(0)
                   *        / \                 / \                      /  \
                   *       /   \               /   \                    /    \
                   *      /     \             /     \                  /      \
                   * L(n-1)(+1)  R(n-3)  ==> LR      R     =>  L(n-2)(-1/0)  N(n-2)(0/+1)
                   *     / \                 / \                     / \        / \
                   *    /   \               /   \                   /   \      /   \
                   *   /     \             /     \                 /     \    /     \
                   * LL(n-3) LR(n-2)(?)   L      LRR           LL(n-3)   LRL LRR   R(n-3)
                   *          / \        / \                            (n-3/n-4)
                   *        LRL LRR     LL  LRL
                   *       (n-3/n-4)
                   *
                   * addition:
                   *    L increased: subtree depth reamins unchanged (n-1)
                   * deletion:
                   *    R decreased: subtree depth decreases from n to n-1
                   */
                  {
                     int bal = node->child[AVL_LEFT]->child[AVL_RIGHT]->balance;
                     AVL_rot(tree, node->child[AVL_LEFT], AVL_LEFT);
                     node = AVL_rot(tree, node, AVL_RIGHT);
                     node->balance = 0;
                     node->child[AVL_LEFT]->balance = 0 - ((bal*(bal+1)) >> 1);// 0/-1 -> 0, 1 -> -1
                     node->child[AVL_RIGHT]->balance = (bal*(bal-1)) >> 1;// 0/1 -> 0, -1 -> 1
                  }
                  if (!add)
                     diff = -1;
                  break;
               default://should not occur
#ifdef AVL_DEBUG
                  AVL_print(node, 10);
#endif
                  break;
            }
            break;
         
         case 2:
            switch (node->child[AVL_RIGHT]->balance) {
               case 1:
                  /*
                   *      N(n)(+2)                  N(n-1)(0)
                   *         / \                       / \
                   *        /   \                     /   \
                   *       /     \                   /     \
                   *    L(n-3)  R(n-1)(1)  ==>     (n-2)(0)  RR(n-2)
                   *              / \               / \
                   *             /   \             /   \
                   *            /     \           /     \
                   *         RL(n-3)  RR(n-2)  L(n-3)   RL(n-3)
                   *
                   * addition:
                   *    R increased: subtree depth remains unchanged (n-1)
                   * deletion:
                   *    L decreased: subtree depth decreases from n to n-1
                   */
                  node = AVL_rot(tree, node, AVL_LEFT);
                  node->balance = 0;
                  node->child[AVL_LEFT]->balance = 0;
                  if (!add)
                     diff = -1;
                  break;
               case 0:
                  /*
                   *      N(n)(+2)                    N(n)(-1)
                   *         / \                       / \
                   *        /   \                     /   \
                   *       /     \                   /     \
                   *    L(n-3)  R(n-1)(0)  ==>     (n-1)(1)  RR(n-2)
                   *              / \               / \
                   *             /   \             /   \
                   *            /     \           /     \
                   *         RL(n-2)  RR(n-2)  L(n-3)   RL(n-2)
                   *
                   * addition:
                   *    R increased: subtree depth increases from n-1 to n
                   * deletion:
                   *    L decreased: subtree remains unchanged (n)
                   */
                  node = AVL_rot(tree, node, AVL_LEFT);
                  node->balance = -1;
                  node->child[AVL_LEFT]->balance = 1;
                  if (add)
                     diff = 1;
                  break;
               case -1:
                  /*
                   *       N(n)(+2)                 N                        RL(n-1)(0)
                   *         / \                   / \                         / \
                   *        /   \                 /   \                       /   \
                   *       /     \               /     \                     /     \
                   *    L(n-3)  R(n-1)(-1) ==>  L       RL       ==>  N(n-2)(-1/0)  R(n-2)(0/+1)
                   *              / \                  / \               / \         / \
                   *             /   \                /   \             /   \       /   \
                   *            /     \              /     \           /     \     /     \
                   *      RL(n-2)(?)  RR(n-3)      RLL      R       L(n-3)   RLL RLR     RR(n-3)
                   *         / \                           / \              (n-3/n-4)
                   *       RLL RLR                       RLR  RR
                   *      (n-3/n-4)
                   *
                   * addition:
                   *    R increased: subtree depth remains unchanged (n-1)
                   * deletion:
                   *    L decreased: subtree depth decreases from n to n-1
                   */
                  {
                     int bal = node->child[AVL_RIGHT]->child[AVL_LEFT]->balance;
                     AVL_rot(tree, node->child[AVL_RIGHT], AVL_RIGHT);
                     node = AVL_rot(tree, node, AVL_LEFT);
                     node->balance = 0;
                     node->child[AVL_LEFT]->balance = 0 - ((bal*(bal+1)) >> 1);// 0/-1 -> 0, 1 -> -1
                     node->child[AVL_RIGHT]->balance = (bal*(bal-1)) >> 1;// 0/1 -> 0, -1 -> 1
                  }
                  if (!add)
                     diff = -1;
                  break;
               default://should not occur
#ifdef AVL_DEBUG
                  AVL_print(node, 10);
#endif
                  break;
            }
            break;

         default://should not occur
#ifdef AVL_DEBUG
            AVL_print(node, 10);
#endif
            break;
      }


      if (diff == 0) // depth of current subtree remains unchanged
         break;
      
      /*
       * update father balance
       */
      if (node->father) {
         node->father->balance += diff * ((node->side << 1) - 1); // += -1/0/+1 * -1(left)/+1(right)
      }

#ifdef AVL_DEBUG
      assert(AVL_check_subtree(node, true) >= 0);
#endif

      node = node->father;
   }
#ifdef AVL_DEBUG
   assert(AVL_check_tree(tree) >= 0);
#endif
}

/*
 * add a node in 'tree' with the given 'key'
 * '*node' must be 
 *    either NULL 
 *    or the future previous/next node
 *    or an ancester of the future previous/next node.
 *    (the 'find' result is fine for example)
 * 
 * return false if key is already in the tree, true if key is added
 * return in '*node' the added or existing node
 */
_AVL_FUNC_ATTRIB bool AVL_add (
      AVL_tree_t *tree, 
#ifdef AVL_DYNPOOL
      AVL_pool_t *pool,
#endif
      AVL_key_t key, 
      AVL_node_t **node)
{

   /*
    * find place to insert
    */
   AVL_node_t *cur = tree->root;
   AVL_node_t *last = NULL;
   enum avl_side_t side = 0;
   
   AVL_debug("AVL_add(%p, 0x%x, %p)\n", tree, key, *node);
   
   if (*node != NULL) {
      cur = *node;
      last = cur->father;
      side = cur->side;
   }
   while (cur != NULL) {
      if (key < cur->key)
         side = AVL_LEFT;
#ifdef AVL_RANGED
      else if (key > cur->max)
#else
      else if (key > cur->key)
#endif
         side = AVL_RIGHT;
      else
         break;
      last = cur;
      cur = cur->child[side];
   }

   /*
    * key is already there
    */
   if (cur != NULL) {
      *node = cur;
      return false;
   }

   /*
    * creating node
    */
#ifdef AVL_DYNPOOL
   cur = AVL_node_create(tree, pool);
#else
   cur = AVL_node_create(tree);
#endif
   cur->father = last;
   cur->child[AVL_LEFT] = NULL;
   cur->child[AVL_RIGHT] = NULL;
   cur->balance  = 0;
   cur->key = key;
#ifdef AVL_RANGED
   cur->max = key;
#endif
   cur->side = side;
   *node = cur;
#ifdef AVL_LINKED
   cur->neighbor[1-side] = last;
#endif

   /*
    * tree was empty
    */
   if (last == NULL) {
#ifdef AVL_LINKED
      cur->neighbor[side] = NULL;
      tree->first = cur;
      tree->last = cur;
#endif
      tree->root = cur;
      return true;
   }

   /*
    * finish neighbor links
    */
#ifdef AVL_LINKED
   cur->neighbor[side] = last->neighbor[side];
   last->neighbor[side] = cur;
   if (cur->neighbor[side])
      cur->neighbor[side]->neighbor[1-side] = cur;
   else {
      if (side == AVL_LEFT)
         tree->first = cur;
      else
         tree->last = cur;
   }
#endif


   /*
    * add in tree and balance 
    */
   last->child[side] = cur;
   last->balance += (2 * side) - 1; // 0 -> -=1, 1 -> += 1

   AVL_balance(tree, last, true);
   
#ifdef AVL_DEBUG
   assert(AVL_check_tree(tree) >= 0);
#endif


   return true;
}


/*
 * add but check the full ranged is free
 * return true if sucessfully added
 * return in '*node' the added node or a problematic node
 */
#ifdef AVL_RANGED
_AVL_FUNC_ATTRIB bool AVL_add_ranged (
      AVL_tree_t *tree, 
#ifdef AVL_DYNPOOL
      AVL_pool_t *pool,
#endif
      AVL_key_t low, 
      AVL_key_t high, 
      AVL_node_t **node)
{
   /*
    * find place to insert
    */
   AVL_node_t *cur = tree->root;
   AVL_node_t *last = NULL;
   enum avl_side_t side = 0;
   
   AVL_debug("AVL_add(%p, 0x%x, %p)\n", tree, key, *node);
   
   if (*node != NULL) {
      cur = *node;
      last = cur->father;
      side = cur->side;
   }
   while (cur != NULL) {
      if (high < cur->key)
         side = AVL_LEFT;
      else if (low > cur->max)
         side = AVL_RIGHT;
      else
         break;
      last = cur;
      cur = cur->child[side];
   }

   /*
    * low key is already there
    */
   if (cur != NULL) {
		*node = cur;
      return false;
   } else if (last != NULL){
	/*
	 * check if there is room for high key
	 */
		if (side == AVL_LEFT) {
			AVL_node_t *neigh = AVL_prev(tree, last);
			if (neigh != NULL && neigh->max >= low) {
				*node = neigh;
				return false;
			}
		} else {
			AVL_node_t *neigh = AVL_next(tree, last);
			if (neigh != NULL && high >= neigh->key) {
				*node = neigh;
				return false;
			}
	  }
	}

   /*
    * creating node
    */
#ifdef AVL_DYNPOOL
   cur = AVL_node_create(tree, pool);
#else
   cur = AVL_node_create(tree);
#endif
   cur->father = last;
   cur->child[AVL_LEFT] = NULL;
   cur->child[AVL_RIGHT] = NULL;
   cur->balance  = 0;
   cur->key = low;
   cur->max = high;
   cur->side = side;
   *node = cur;
#ifdef AVL_LINKED
   cur->neighbor[1-side] = last;
#endif

   /*
    * tree was empty
    */
   if (last == NULL) {
#ifdef AVL_LINKED
      cur->neighbor[side] = NULL;
      tree->first = cur;
      tree->last = cur;
#endif
      tree->root = cur;
      return true;
   }

   /*
    * finish neighbor links
    */
#ifdef AVL_LINKED
   cur->neighbor[side] = last->neighbor[side];
   last->neighbor[side] = cur;
   if (cur->neighbor[side])
      cur->neighbor[side]->neighbor[1-side] = cur;
   else {
      if (side == AVL_LEFT)
         tree->first = cur;
      else
         tree->last = cur;
   }
#endif


   /*
    * add in tree and balance 
    */
   last->child[side] = cur;
   last->balance += (2 * side) - 1; // 0 -> -=1, 1 -> += 1

   AVL_balance(tree, last, true);
   
#ifdef AVL_DEBUG
   assert(AVL_check_tree(tree) >= 0);
#endif


   return true;
}
#endif

/*
 * remove a node
 */

_AVL_FUNC_ATTRIB void AVL_rem (
      AVL_tree_t *tree, 
#ifdef AVL_DYNPOOL
      AVL_pool_t *pool,
#endif
      AVL_node_t *node)
{   

   AVL_node_t *bal = NULL;
   int diff = 0; 
   
   AVL_debug("AVL_rem(%p, %p(%x))\n", tree, node, node->key);
   
   if (node->child[AVL_LEFT] != NULL || node->child[AVL_RIGHT] != NULL) {
      
      /*
       * find the nearest node 'repl' in the bigest subtree of node
       * this node is either a leaf or has one leaf child
       * since we will remove node we can replace node by repl 
       * in the tree
       */
      enum avl_side_t side;
      if (node->balance < 0)
         side = AVL_LEFT;
      else
         side = AVL_RIGHT;
      enum avl_side_t invside = 1 - side;
      AVL_node_t *repl = node->child[side];
      // repl is not null, since node has at least one child
      // and we take a child in its deepest subtree
      while (repl->child[invside] != NULL)
         repl = repl->child[invside];

      /*
       * remove repl from the tree
       * and replace it by its child (if it has one)
       * (which is (and stay) a leaf)
       * note that 'repl->father' may be 'node'
       */
      repl->father->child[repl->side] = repl->child[side];
      if (repl->child[side]) {
         repl->child[side]->father = repl->father;
         repl->child[side]->side = repl->side;
      }
      repl->father->balance += 1 - (repl->side << 1);// 0 -> +=1, 1 -> -=1
      bal = (repl->father == node) ? repl : repl->father;

      /*
       * replace node by repl in the tree
       */
      repl->father = node->father;
      repl->child[AVL_LEFT] = node->child[AVL_LEFT];
      repl->child[AVL_RIGHT] = node->child[AVL_RIGHT];
      repl->balance = node->balance;
      repl->side = node->side;
      if (repl->child[AVL_LEFT])
         repl->child[AVL_LEFT]->father = repl;
      if (repl->child[AVL_RIGHT])
         repl->child[AVL_RIGHT]->father = repl;
   
      if (node->father)
         node->father->child[node->side] = repl;
      else
         tree->root = repl;

   } else {
      bal = node->father;
      if (node->father) {
         node->father->child[node->side] = NULL;
         node->father->balance += diff = 1 - (node->side << 1); // 0 -> +=1, 1 -> -=1
      } else {
         tree->root = NULL;
      }
   }
   
   /* remove neighbor links */
#ifdef AVL_LINKED
   if (node->neighbor[AVL_LEFT])
      node->neighbor[AVL_LEFT]->neighbor[AVL_RIGHT] = node->neighbor[AVL_RIGHT];
   else
      tree->first = node->neighbor[AVL_RIGHT];
   if (node->neighbor[AVL_RIGHT])
      node->neighbor[AVL_RIGHT]->neighbor[AVL_LEFT] = node->neighbor[AVL_LEFT];
   else
      tree->last = node->neighbor[AVL_LEFT];
#endif

   /* remove node and balance */
#ifdef AVL_DYNPOOL
   AVL_node_delete(tree, pool, node);
#else
   AVL_node_delete(tree, node);
#endif
   AVL_balance(tree, bal, false);

   return;
}

#undef AVL_prefix 
#undef AVL_key_t
#ifdef AVL_data_t
#undef AVL_data_t
#endif

#undef AVL_tree_t 
#undef AVL_node_t 
#undef AVL_pool_t 
#undef AVL_array_t 

#undef AVL_pool_create 
#undef AVL_pool_delete 
#undef AVL_pool_init 
#undef AVL_pool_term
#undef AVL_node_create 
#undef AVL_node_delete 
#undef AVL_init        
#undef AVL_reset       
#undef AVL_first       
#undef AVL_last        
#undef AVL_prev        
#undef AVL_next        
#undef AVL_find        
#undef AVL_add         
#undef AVL_add_ranged  
#undef AVL_rem         
#undef AVL_balance     
#undef AVL_rot
#undef AVL_print
#undef AVL_print_r
#undef AVL_check_subtree
#undef AVL_check_tree

#undef _AVL_FUNC_ATTRIB
#undef AVL_POOL_ARRAY_SIZE
#undef AVL_CONCAT
#undef AVL_XCONCAT

#ifdef AVL_RANGED
#undef AVL_RANGED
#endif

#ifdef AVL_LINKED
#undef AVL_LINKED
#endif

#ifdef AVL_EXTERN
#undef AVL_EXTERN
#endif

#ifdef AVL_DEBUG
#undef AVL_DEBUG
#endif

#undef AVL_PRINT_OUTPUT

#ifdef AVL_PRINT_DATA
#undef AVL_PRINT_DATA
#endif

#ifdef AVL_STAT
#undef AVL_STAT
#endif

#ifdef AVL_DYNPOOL
#undef AVL_DYNPOOL
#endif

#ifdef AVL_PRINT
#undef AVL_PRINT
#endif
