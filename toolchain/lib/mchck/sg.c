#include <mchck.h>

struct sg *
sg_get_next(struct sg *sg)
{
        if (sg == NULL)
                return (NULL);
        if (sg->flags & SG_MORE)
                return (sg + 1);
        return (NULL);
}

struct sg *
sg_init1(struct sg *sg, uint8_t *buf, uint16_t len)
{
        *sg = (struct sg){
                .data = buf,
                .len = len
        };
        return (sg);
}

struct sg *
sg_init_list(struct sg *sg, size_t elems, ...)
{
        struct sg *cursg;
        va_list ap;

        va_start(ap, elems);
        cursg = sg;
        while (elems > 0) {
                sg_init1(cursg, va_arg(ap, uint8_t *), va_arg(ap, int));
                if (--elems > 0) {
                        cursg->flags |= SG_MORE;
                        cursg = sg_get_next(cursg);
                }
        }
        va_end(ap);
        return (sg);
}


struct sg *
sg_simplify(struct sg *sg)
{
        while (sg && sg->len == 0)
                sg = sg_get_next(sg);
        return (sg);
}

static size_t
sg_total_length1(struct sg *sg, size_t len)
{
        if (sg == NULL)
                return (len);
        return (sg_total_length1(sg_get_next(sg), len + sg->len));
}

size_t
sg_total_length(struct sg *sg)
{
        return (sg_total_length1(sg, 0));
}

uint8_t *
sg_data(struct sg *sg)
{
        return (sg->data);
}

enum sg_move
sg_move(struct sg **psg, uint16_t amount)
{
        struct sg *sg = *psg;

        if (sg == NULL)
                return (SG_END);

        if (sg->len < amount)
                amount = sg->len;

        if (sg->data != NULL)
                sg->data += amount;
        sg->len -= amount;

        if (sg->len == 0) {
                *psg = sg_get_next(sg);
                if (*psg == NULL)
                        return (SG_END);
                else
                        return (SG_NEXT);
        }
        return (SG_SAME);
}
