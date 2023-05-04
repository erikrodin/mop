/*
 * 	startup.c
 *
 */
__attribute__((naked)) __attribute__((section (".start_section")) )
void startup ( void )
{
__asm__ volatile(" LDR R0,=0x2001C000\n");		/* set stack */
__asm__ volatile(" MOV SP,R0\n");
__asm__ volatile(" BL main\n");					/* call main */
__asm__ volatile(".L1: B .L1\n");				/* never return */
}

void app_init(void) 
{
#define PORT_D_BASE 0x40020C00
#define portD_Moder ((volatile unsigned int* ) (PORT_D_BASE))
#define portD_Otyper ((volatile unsigned short*) (PORT_D_BASE+0x4))
#define portD_Ospeedr ((volatile unsigned int* ) (PORT_D_BASE+0x8))
#define portD_Pupdr ((volatile unsigned int*) (PORT_D_BASE+0xC))
#define portD_IdrLow ((volatile unsigned char* ) (PORT_D_BASE+0x10))
#define GPIO_IDR_HIGH ((volatile unsigned char*) (PORT_D_BASE+0x11))
#define portD_OdrLow ((volatile unsigned char* ) (PORT_D_BASE+0x14))
#define GPIO_ODR_HIGH ((volatile unsigned char*) (PORT_D_BASE+0x15))
#define STK_CTRL ((volatile unsigned int *)(0xE000E010))
#define STK_LOAD  ((volatile unsigned int *)(0xE000E014))
#define STK_VAL ((volatile unsigned int *)(0xE000E018))



*portD_Moder = 0x55005555;
*portD_Pupdr = 0x00AA0000;
*portD_Otyper = 0x00000000;
}
unsigned char keyb (void) {
    unsigned char key[]={1,2,3,0xA,4,5,6,0xB,7,8,9,0xC,0xE,0,0xF,0xD}; 
    int row, col;
    for (row=1; row <=4 ; row++ ) {
        kbdActivate( row );
        if( (col = kbdGetCol () ) ){
            return key [4*(row-1)+(col-1) ]; 
        }
    }
*GPIO_ODR_HIGH = 0;
return 0xFF;
}


    void kbdActivate( unsigned int row ){
/* Aktivera angiven rad hos tangentbordet, eller deaktivera samtliga */ switch( row )
    {
    case 1: *GPIO_ODR_HIGH = 0x10; break;
    case 2: *GPIO_ODR_HIGH = 0x20; break; 
    case 3: *GPIO_ODR_HIGH = 0x40; break;   
    case 4: *GPIO_ODR_HIGH = 0x80; break; 
    case 0: *GPIO_ODR_HIGH = 0x00; break;
    }
}

    int kbdGetCol ( void )
{ /* Om någon tangent (i aktiverad rad) är nedtryckt, returnera dess kolumnnummer, * annars, returnera 0 */
unsigned char c = *GPIO_IDR_HIGH; 
if ( c & 0x8 ) return 4;
if ( c & 0x4 ) return 3;
if ( c & 0x2 ) return 2;
if ( c & 0x1 ) return 1;
return 0;
}

void out7seg(unsigned char c){ 
    static unsigned char segCode[]={0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x67,0x77,0x7C,0x39,0x5E,0x79,0x71 };

    if( c > 15){
        *portD_OdrLow = 0;
        return;
        }
    *portD_OdrLow = segCode[c];
    }

__attribute__((naked))
void graphic_initalize(void)
{
__asm volatile (" .HWORD 0xDFF0\n");
__asm volatile (" BX LR\n");
}
__attribute__((naked))
void graphic_clear_screen(void)
{
__asm volatile (" .HWORD 0xDFF1\n");
__asm volatile (" BX LR\n");
}
__attribute__((naked))
void graphic_pixel_set( int x, int y)
{
__asm volatile (" .HWORD 0xDFF2\n");
__asm volatile (" BX LR\n");
}

__attribute__((naked))
void graphic_pixel_clear( int x, int y)
{
__asm volatile (" .HWORD 0xDFF3\n");
__asm volatile (" BX LR\n");
}

void delay_250ns( void )
{
    *STK_CTRL = 0;
    *STK_LOAD = ( (168/4) -1 ); 
    *STK_VAL = 0;
    *STK_CTRL = 5;
    while( (*STK_CTRL & 0x10000 )== 0 ); 
    *STK_CTRL = 0;
}
void delay_micro(unsigned int us) 
{
#ifdef  SIMULATOR
        us = us / 1000; 
        us++;
#endif
    while( us > 0 )
    {
    delay_250ns();
    delay_250ns(); 
    delay_250ns(); 
    delay_250ns(); 
    us--;
    } 
}

 void delay_milli(unsigned int ms)
 {
#ifdef SIMLUTAOR
     ms++;
#endif
     while( ms > 0 )
    {
    delay_250ns();
    delay_250ns(); 
    delay_250ns(); 
    delay_250ns(); 
    ms--;
    } 
 }
 
 typedef struct tPoint
{
    unsigned char x;
    unsigned char y;
} POINT;

#define MAX_POINTS 30
typedef struct tGeometry
{
    int numpoints;
    int sizex;
    int sizey;
    POINT px[ MAX_POINTS ];
} GEOMETRY, *PGEOMETRY;

typedef struct tObj {
    PGEOMETRY geo;
    int dirx, diry;
    int posx, posy;
    void (*draw)(struct tObj *);
    void (*clear)(struct tObj *);
    void (*move)(struct tObj *);
    void (*set_speed)(struct tObj *, int, int);
} OBJECT, *POBJECT;

void draw_ballobject(POBJECT o)
{
    for(int p=0; p<(o->geo)->numpoints; p++)
    {
        char x,y;
        x=(o->geo)->px[p].x + o->posx;
        y=o->geo->px[p].y + o->posy;
        graphic_pixel_set(x,y);
    }
}
    
void clear_ballobject(POBJECT o)
{
     for(int p=0; p<(o->geo)->numpoints; p++)
    {
        char x,y;
        x=(o->geo)->px[p].x + o->posx;
        y=o->geo->px[p].y + o->posy;
        graphic_pixel_clear(x,y);
    }
}


void draw_paddelobject(POBJECT o )
{   
    for(int p=0; p<(o->geo)->numpoints; p++)
    {
        char x,y;
        x=(o->geo)->px[p].x + o->posx;
        y=o->geo->px[p].y + o->posy;
        graphic_pixel_set(x,y);
        
    }
}

void clear_paddelobject(POBJECT o )
{
    for(int p=0; p<(o->geo)->numpoints; p++)
    {
        char x,y;
        x=(o->geo)->px[p].x + o->posx;
        y=o->geo->px[p].y + o->posy;
        graphic_pixel_clear(x,y);
    }
}

void move_paddelobject(POBJECT o)
{
    clear_paddelobject(o);
    o->posy = o->posy + o->diry;
    if((o->posy)<1)
    {
        o->diry=0;
		o->posy=1;
    }
    if ((o->posy + o->geo->sizey)>64)
    {
       o->diry=0;
	   o->posy=64 - o->geo->sizey;
    }
	else{
		o->diry=0;
		
	}
    draw_paddelobject(o);
}

void set_ballobject_speed(POBJECT o, int dirx, int diry)
{
    o->dirx=dirx;
    o->diry=diry;
}

void set_paddelobject_speed(POBJECT o, int dirx, int diry)
{
    o->diry=diry;
    o->dirx=dirx;
}

GEOMETRY ball_geometry = 
{ 
 12, /* numpoints */ 
 4,4, /* sizex,sizey */ 
 { 
 /* px[0,1,2 ...] */ 
 {0,1},{0,2},{1,0},{1,1},{1,2},{1,3},{2,0},{2,1},{2,2},{2,3},{3,1},{3,2} 
 } 
};

GEOMETRY paddel_geometry =
{
27,
5,9,
{
    {0,0},{1,0},{2,0},{3,0},{4,0},{0,1},{0,2},{0,3},{0,4},{0,5},{0,6},{0,7},{0,8},{1,8},{2,8},{3,8},{4,8},{4,7},{4,6},{4,5},{4,6},{4,5},{4,4},{4,3},{4,2},{4,1},{2,3},{2,4},{2,5} 
    }
};

static OBJECT paddelobject=
{ 
    &paddel_geometry,                      /* geometri för en boll */ 
    0,0,                               /* initiala riktningskoordinater */ 
    120,25,                                 /* initial startposition */ 
    draw_paddelobject, 
    clear_paddelobject, 
    move_paddelobject, 
    set_paddelobject_speed 
};

void move_ballobject(POBJECT o)
{
    POBJECT t = &paddelobject;
    int k=0;
    clear_ballobject(o);
    o->posx= o->posx + o ->dirx;
    o->posy = o->posy + o->diry;
    if((o->posx)<1)
    {
        o->dirx=-(o->dirx);
    }
    if ((o->posx)>120)
    {
        for(int p=0; p<(o->geo)->numpoints && k==0; p++)
        {
            for(int w=0; w<(t->geo)->numpoints && k==0; w++)
            {
                if(o->posy+(o->geo)->px[p].y==t->posy+(t->geo)->px[w].y)
                {
                    o->dirx=-(o->dirx);
                    k=1;
                    
                }
                
            }
        }
    }
    if((o->posy)<1)
    {
        o->diry=-(o->diry);
    }
    if ((o->posy + o->geo->sizey)>64)
    {
       o->diry=-(o->diry);
    }
    draw_ballobject(o);
}

static OBJECT ballobject=
{ 
    &ball_geometry,                      /* geometri för en boll */ 
    4,1,                                 /* initiala riktningskoordinater */ 
    1,1,                                 /* initial startposition */ 
    draw_ballobject, 
    clear_ballobject, 
    move_ballobject, 
    set_ballobject_speed 
};


int main(void) 
{ 
    char c; 
    POBJECT p = &ballobject; 
    POBJECT t = &paddelobject;
    app_init(); 
    graphic_initalize(); 
    graphic_clear_screen (); 
    while( 1 ) 
    { 
        p->move (p);
        t->move( t ); 
        delay_milli(20); 
        c = keyb(); 
        switch( c ) 
        { 
            case 5: t->set_speed( t, 0, 0); break; 
            case 2: t->set_speed( t, 0, 3); break; 
            case 8: t->set_speed( t, 0,-3); break; 
        }
		
    }
}
