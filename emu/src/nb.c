
//Atlas renderer

//To render
/*
    Raw Data is a R16 Window resolution is 240*4 160*4
    The fragment shader reads each r16 block as a texel. 
    RA_B_C_D  
    x,y     x+1,y
    ----------
    | A | B |
    ---------
    | C | D |
    x,y+1   x+1,y+1
    Where x,y is position in NB Screen space
    Each 4 bit values maps into the palette. 
    

*/
//Map x [A,B] to Y[C,D] 
//Y = (X-A)/(B-A) * (D-C) + C
