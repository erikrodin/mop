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

void init_app(void)
{

#define STK_CTRL ((volatile unsigned int *)(0xE000E010))
#define STK_LOAD  ((volatile unsigned int *)(0xE000E014))
#define STK_VAL ((volatile unsigned int *)(0xE000E018))

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

typedef struct COORD{ int x,y } COORD;

typedef struct
{
	COORD start,end;
}LINE, *PLINE;
 
typedef struct polygonpoint
{
    char x,y;
    struct polygonpoint *next;
} POLYPOINT, *PPOLYPOINT;
 

int draw_polygon(PPOLYPOINT p)
{
    POLYPOINT p0;
    POLYPOINT p1;
    PLINE l;
    p0.x = p->x;
    p0.y = p->y;
    PPOLYPOINT ptr = p->next;
    while(ptr!=0)
    {
        p1.x=ptr->x;
        p1.y=ptr->y;
        l->start.x=p0.x;
        l->start.y=p0.y;
        l->end.x=p1.x;
        l->end.y=p1.y;
        draw_line(l);
        p0.x=ptr->x;
        p0.y=ptr->y;
        ptr=ptr->next;
    }
    
}

 
int draw_line(PLINE l)
{
	int y;
	int x;
	int ystep;
	int x0=l->start.x;
	int x1=l->end.x;
	int y0=l->start.y;
	int y1=l->end.y;
	int steep;
	if (abs(y1-y0) > abs(x1-x0))
		steep=1;
	else
		steep=0;
	if (steep==1)
	{
		swap(&x0,&y0);
		swap(&x1,&y1);
		
	}
	if (x0 > x1)
	{
		swap(&x0,&x1);
		swap(&y0,&y1);
		
	}
	int deltax = x1 - x0;
	int deltay = abs(y1 - y0);
	int error = 0;
	y=y0;
	if (y0<y1)
    {
        ystep=1;
        
    }
    else
    {
        ystep=-1;
        
    }
    for (x=x0; x<=x1; x++)
    {
        if (steep)
        {
            graphic_pixel_set(y,x);
            
        }
        else
        {
            graphic_pixel_set(x,y);
        }
        error= error + deltay;
        
        
        if (2*error>=deltax)
        {
            y = y + ystep;
            error= error - deltax;
        }
    }
}

void swap(int *x, int *y)
{
    int swp;
    swp=*x;
    *x=*y;
    *y=swp;
}

POLYPOINT pg8 = {20,20, 0};
POLYPOINT pg7 = {20,55, &pg8};
POLYPOINT pg6 = {70,60, &pg7};
POLYPOINT pg5 = {80,35, &pg6};
POLYPOINT pg4 = {100,25, &pg5};
POLYPOINT pg3 = {90,10, &pg4};
POLYPOINT pg2 = {40,10, &pg3};
POLYPOINT pg1 = {20,20, &pg2};


void main(void)
{
    int i;
	graphic_initalize();
    graphic_clear_screen();
    while(1)
    
    {
        draw_polygon( &pg1);
        delay_milli(5);
    }
    graphic_clear_screen();
}