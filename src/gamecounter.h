#ifndef GAMECOUNTER_H
#define GAMECOUNTER_H

struct gamecounter_t {
        
};

void gamecounter_t_init(struct gamecounter_t* gc);
void gamecounter_t_terminate(struct gamecounter_t* gc);

struct gamecounter_t* gamecounter_t_create();
void gamecounter_t_destroy(struct gamecounter_t* gc);

#endif /* GAMECOUNTER_H */
