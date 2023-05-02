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

#define PORT_D_BASE 0x40020C00
#define portD_Moder ((volatile unsigned int* ) (PORT_D_BASE))
#define portD_Otyper ((volatile unsigned short*) (PORT_D_BASE+0x4))
#define portD_Ospeedr ((volatile unsigned int* ) (PORT_D_BASE+0x8))
#define portD_Pupdr ((volatile unsigned int*) (PORT_D_BASE+0xC))
#define portD_IdrLow ((volatile unsigned char* ) (PORT_D_BASE+0x10))
#define GPIO_IDR_HIGH ((volatile unsigned char*) (PORT_D_BASE+0x11))
#define portD_OdrLow ((volatile unsigned char* ) (PORT_D_BASE+0x14))
#define GPIO_ODR_HIGH ((volatile unsigned char*) (PORT_D_BASE+0x15))

//-------------------------------------------------------------------------------

#define PORT_DISPLAY_BASE 0x40021000 /* MD407 port E */
#define portModer ((volatile unsigned int *) (PORT_DISPLAY_BASE))
#define portOtyper ((volatile unsigned short *)(PORT_DISPLAY_BASE+0x4))
#define portOspeedr ((volatile unsigned int *) (PORT_DISPLAY_BASE+0x8))
#define portPupdr ((volatile unsigned int *)(PORT_DISPLAY_BASE+0xC))
#define portIdr ((volatile unsigned short *)(PORT_DISPLAY_BASE+0x10))
#define portIdrLow ((volatile unsigned char *)(PORT_DISPLAY_BASE+0x10))
#define portIdrHigh ((volatile unsigned char *)(PORT_DISPLAY_BASE+0x10+1))
#define portOdrLow ((volatile unsigned char *)(PORT_DISPLAY_BASE+0x14))
#define portOdrHigh ((volatile unsigned char *)(PORT_DISPLAY_BASE+0x14+1))

//------------------------------------------------------------------------------

#define STK_CTRL ((volatile unsigned int *)(0xE000E010))
#define STK_LOAD  ((volatile unsigned int *)(0xE000E014))
#define STK_VAL ((volatile unsigned int *)(0xE000E018))

#define B_E 0x40 /* enable */               //ascii display
#define B_SELECT 4 /* välj ascii/lcd */
#define B_RW 2 /* read/write */
#define B_RS 1 /* register select */

void delay_1mikro();
void delay();
void delay_micro();
void delay_milli();
/**portD_Moder = 0x55005555;
*portD_Pupdr = 0x00AA0000;
*portD_Otyper = 0x00000000;

*portModer = 0x55005555;
*portPupdr = 0x00AA0000;
*portOtyper = 0x00000000;
*/
void app_init(void) 
{

    *portD_Moder &= 0x00000000; /*Reset bit 31-16      Part of exercise 4.1*/
    *portD_Moder |= 0x55005555; /*Config bit 31-16 to "01-00" */
    
    *portD_Otyper &= 0x0FFF; /*Reset 11-0 and set bit 15-12 to 0 */
    
    *portD_Pupdr &= 0x0000FFFF; /*Reset bit 31-16 */
    *portD_Pupdr |= 0x00AA0000; /*Set bit 31-24 to 0 and bit 23-16 to A... "10"*/
    *portModer = 0x55555555; /*initiering av port E, sätter port E0-7 och E8-15*/
}
//---------------------------------------




//-----------------------------------

void ascii_ctrl_bit_set( unsigned char x )
{
    unsigned char c;
    c = *portOdrLow;
    c |= ( B_SELECT | x );
    *portD_OdrLow = c;
}

void ascii_ctrl_bit_clear ( unsigned char x )
{
    unsigned char c;
    c = *portOdrLow;
    c &= ( B_SELECT | ~x );
    *portOdrLow = c;
    
}
void ascii_write_controller( unsigned char c )
{
    delay_250ns();
    ascii_ctrl_bit_set( B_E );
    *portOdrHigh = c;
    delay_250ns();
    ascii_ctrl_bit_clear( B_E );
    delay_250ns();
}
void ascii_write_cmd( unsigned char command )
{
    ascii_ctrl_bit_clear( B_RS );            /*sätter till 0 */
    ascii_ctrl_bit_clear( B_RW );            /*sätter till 0 */
    ascii_write_controller(command);
}
        
void ascii_write_data( data )
{
    ascii_ctrl_bit_set( B_RS );     /*sätter till 1 */
    ascii_ctrl_bit_clear( B_RW );   /*sätter till 0 */
    ascii_write_controller(data);
}

unsigned char ascii_read_controller( unsigned char c  )
{
    ascii_ctrl_bit_set( B_E );
    delay_250ns();
   
    c = *portIdrHigh;
    ascii_ctrl_bit_clear( B_E );
    return c;
}
unsigned char ascii_read_status( unsigned char c)
{   
    
    *portModer = 0x00005555;
    ascii_ctrl_bit_clear( B_RS );
    ascii_ctrl_bit_set( B_RW );
    c= ascii_read_controller;
    *portModer = 0x55555555;
    return c;
}  
        
unsigned char ascii_read_data( unsigned char c  )
{    *portModer = 0x00005555;
     ascii_ctrl_bit_set( B_RS );
     ascii_ctrl_bit_set( B_RW );
     c= ascii_read_controller;
     *portModer = 0x55555555;
    return c;
}  

 void ascii_init()
 
    {
        ascii_wait_ready();
        ascii_write_cmd( 0x38 ); /* Function set */
        delay_micro( 39 );
        ascii_wait_ready(); 
        ascii_write_cmd( 0x0C ); /* Display on */
        delay_micro( 39 );
        ascii_wait_ready();
        ascii_write_cmd( 1 ); /* Clear display */
        delay_milli( 2 );
        ascii_wait_ready();
        ascii_write_cmd( 6 ); /* Entry mode set */
        delay_micro( 39 );
    }
    
    
    void ascii_write_char( char c)
    {
    ascii_wait_ready();
    ascii_write_data( c );
    delay_micro( 39 );

    }

    void ascii_gotoxy( int x, int y)
 {
       int address=x-1;
    if (y==2)
     {   (address= address +0x40);
     } 
    
        { //(address=x-1);
        ascii_write_cmd( 0x80 | address);
       }
 }


    void ascii_wait_ready(command)
    {
    while( ( ascii_read_status(command) & 0x80)== 0x80 );
    delay_micro( 8 );
   // ascii_write_cmd( 1 );
    delay_milli( 2 );
    }

//----------------------------------------------------------------------------
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
unsigned char i = *GPIO_IDR_HIGH; 
if ( i & 0x8 ) return 4;
if ( i & 0x4 ) return 3;
if ( i & 0x2 ) return 2;
if ( i & 0x1 ) return 1;
return 0;
}

void out7seg(unsigned char i){ 
    static unsigned char segCode[]={0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x67,0x77,0x7C,0x39,0x5E,0x79,0x71 };

    if( i> 15){
        *portD_OdrLow = 0;
        return;
        }
    *portD_OdrLow = segCode[i];
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

#define MAX_POINTS 40
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



void set_ballobject_speed(POBJECT o, int dirx, int diry)
{
    o->dirx=dirx;
    o->diry=diry;
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


GEOMETRY spider_geometry =
{
    22,
    6,8,
    {
        {2,0},{3,0},{1,1},{4,1},{0,2},{1,2},{2,2},{3,2},{4,2},{5,2},{0,3},
        {2,3},{3,3},{5,3},{1,4},{4,4},{2,5},{3,5},{1,6},{4,6},{0,7},{5,7}
    }
};


void move_ballobject(POBJECT o)
{
    //POBJECT t = &spindelobject;
    clear_ballobject(o);
    o->posx= o->posx + o ->dirx;
    o->posy = o->posy + o->diry;
    if ((o->posx)<1)
    {
        o->dirx= -(o->dirx);
    } 
    if ((o->posx + o ->geo->sizex)>128)
    {
        o->dirx=-(o->dirx);
    }
     if ((o->posy)<1)
    {
        o->diry= -(o->diry);
    } 
    if ((o->posy + o ->geo->sizey)>64)
    {
        o->diry=-(o->diry);
    }
    draw_ballobject(o);
}



void draw_spindelobject(POBJECT o )
{   
    for(int p=0; p<(o->geo)->numpoints; p++)
    {
        char x,y;
        x=(o->geo)->px[p].x + o->posx;
        y=o->geo->px[p].y + o->posy;
        graphic_pixel_set(x,y);
        
    }
}

int objects_overlap(POBJECT o1, POBJECT o2)
{    
    int k=0;
    {
        for(int p=0; p<(o1->geo)->numpoints && k==0; p++)
        {
            for(int w=0; w<(o2->geo)->numpoints && k==0; w++)
            {
                if(o1->posy+(o1->geo)->px[p].y==o2->posy+(o2->geo)->px[w].y && o1->posx+(o1->geo)->px[p].x==o2->posx+(o2->geo)->px[w].x)
                {
                    return 13;
                    k=1;
                    
                }
                
                
            }
        }
    }
}


void move_spindelobject(POBJECT o)
{
    clear_spindelobject(o);
    o->posy = o->posy + o->diry;
    o->posx = o->posx + o->dirx;
    
    draw_spindelobject(o);
}

void clear_spindelobject(POBJECT o)
{
    for(int p=0; p<(o->geo)->numpoints; p++)
    {
        char x,y;
        x=(o->geo)->px[p].x + o->posx;
        y=o->geo->px[p].y + o->posy;
        graphic_pixel_clear(x,y);
    }
}

void set_spindelobject_speed(POBJECT o, int dirx, int diry)
{
    o->diry=diry;
    o->dirx=dirx;
}

static OBJECT spindelobject=
{ 
    &spider_geometry,                      /* geometri för en boll */ 
    0,0,                                 /* initiala riktningskoordinater */ 
    55,28,                                 /* initial startposition */ 
    draw_spindelobject, 
    clear_spindelobject, 
    move_spindelobject, 
    set_spindelobject_speed 
};

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


//------------------------------------

void asciidisplay (void)
{ 
        char *s;
        char test1[] = "Timer ";
        char test2[] =  "Display - test";
        
        app_init();
        ascii_init();
        ascii_gotoxy(1,1);
        s= test1;
        while( *s )
            ascii_write_char( *s++ );
            ascii_gotoxy(1,2);
        s= test2;
        while( *s )
            ascii_write_char( *s++ );
}





int main(int argc, char **argv) 
{ 
 char i;
 
 POBJECT victim = &ballobject;
 POBJECT creature = &spindelobject; 
 app_init(); 
// asciidisplay();
 graphic_initalize(); 
 graphic_clear_screen(); 
 victim->set_speed( victim, 4, 1); 
 while( 1 ) 
 { 
     
 victim->move( victim );
 creature->move( creature ); 
 i = keyb(); 
 switch( i ) 
 { 
 case 6: creature->set_speed( creature, 2, 0); break; 
 case 4: creature->set_speed( creature, -2, 0); break; 
 case 5: creature->set_speed( creature, 0, 0); break; 
 case 2: creature->set_speed( creature, 0, -2); break; 
 case 8: creature->set_speed( creature, 0, 2); break; 
 default: 
 creature->set_speed( creature, 0, 0); break; 
 } 
 if( objects_overlap( victim, creature )==13) 
 { 
 // Game over 
 break; 
 } 
 

 delay_milli(20); 
 } 
 
  
}