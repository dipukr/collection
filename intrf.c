#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Step 1: Define the interface as a struct of function pointers */
typedef struct ShapeVTable {
    double (*area)(void *self);
} ShapeVTable;


/* Step 2: Define the base "interface" type */
typedef struct Shape {
    ShapeVTable *vtable;
} Shape;

/* Step 3: Circle "class" */
typedef struct {
    Shape base;  // must be first to allow upcasting
    double radius;
} Circle;

/* Circle methods */
double Circle_area(void *self) {
    Circle *c = self;
    return M_PI * c->radius * c->radius;
}

/* Circle vtable (static and shared by all Circle objects) */
ShapeVTable circle_vtable = {
    .area = Circle_area
};

/* Constructor */
Circle *Circle_new(double r) {
    Circle *c = malloc(sizeof(Circle));
    c->base.vtable = &circle_vtable;
    c->radius = r;
    return c;
}

/* Step 4: Rectangle "class" */
typedef struct {
    Shape base;
    double width, height;
} Rectangle;

double Rectangle_area(void *self) {
    Rectangle *r = self;
    return r->width * r->height;
}

ShapeVTable rectangle_vtable = {
    .area = Rectangle_area
};

Rectangle *Rectangle_new(double w, double h) {
    Rectangle *r = malloc(sizeof(Rectangle));
    r->base.vtable = &rectangle_vtable;
    r->width = w;
    r->height = h;
    return r;
}

/* Step 5: Polymorphic behavior */
void printShapeInfo(Shape *s) {
    printf("Area: %.2f, Perimeter: %.2f\n",
           s->vtable->area(s),
           s->vtable->perimeter(s));
}

int main() {
    Shape *s1 = (Shape *) Circle_new(5.0);
    Shape *s2 = (Shape *) Rectangle_new(4.0, 6.0);

    printShapeInfo(s1);
    printShapeInfo(s2);

    free(s1);
    free(s2);
    return 0;
}
