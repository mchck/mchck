/*
 * Scatter/gather I/O
 *
 * This is an abstraction for expressing scatter/gather I/O operations.
 * A scatter/gather I/O transaction is a set of sequential transfers
 * to/from different buffers.
 *
 */

struct sg {
        uint8_t *data;
        uint16_t len;
        enum sg_flags {
                SG_MORE   = 1 << 0,
                SG__USER1 = 1 << 1,
        } flags;
};

enum sg_move {
        SG_END,
        SG_SAME,
        SG_NEXT
};

/*
 * Retrieve the next transfer of the transaction.
 */
struct sg *sg_get_next(struct sg *sg);

/*
 * Initialize a scatter/gather transaction with a single transfer
 */
struct sg *sg_init1(struct sg *sg, uint8_t *buf, uint16_t len);

/*
 * Initialize a scatter/gather transaction multiple transfers.
 * elems is the number of transfers. sg should point to an array of
 * `struct sg` of length `elems`. The varargs are a buffer/transfer
 * length pairs.
 */
struct sg *sg_init_list(struct sg *sg, size_t elems, ...);

/*
 * Remove zero-length transfers from transaction.
 */
struct sg *sg_simplify(struct sg *sg);

/*
 * Compute the total length of all transfers in a transaction.
 */
size_t sg_total_length(struct sg *sg);

/*
 * Retrieve a pointer to the next byte of the transaction.
 */
uint8_t *sg_data(struct sg *sg);

/*
 * Advance the pointer to the next byte of the transaction.
 */
enum sg_move sg_move(struct sg **sg, uint16_t amount);

__attribute__((__always_inline__))
inline struct sg *
sg_init(struct sg *sg, void *buf, uint16_t len, ...)
{
        return (sg_init_list(sg, __builtin_va_arg_pack_len() / 2 + 1,
                             buf, len,
                             __builtin_va_arg_pack()));
}
