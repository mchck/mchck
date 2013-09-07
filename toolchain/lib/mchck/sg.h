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

struct sg *sg_get_next(struct sg *sg);
struct sg *sg_init1(struct sg *sg, uint8_t *buf, uint16_t len);
struct sg *sg_init_list(struct sg *sg, size_t elems, ...);
struct sg *sg_simplify(struct sg *sg);
size_t sg_total_lengh(struct sg *sg);
uint8_t *sg_data(struct sg *sg);
enum sg_move sg_move(struct sg **sg, uint16_t amount);

__attribute__((__always_inline__))
inline struct sg *
sg_init(struct sg *sg, void *buf, uint16_t len, ...)
{
        return (sg_init_list(sg, __builtin_va_arg_pack_len() / 2 + 1, buf, len, __builtin_va_arg_pack()));
}
