// [config]
// expect_result: pass
// glsl_version: 1.10
// [end config]
//
// This test checks how the preprocessor handles defines which has trailing spaces

//Define without trailing space and redefine with training spaces
#define A1 1
#define A1 1 

#define A2 1
#define A2 1  

#define A3 1
#define A3 1   

//Define with trailing space and redefine with no training spaces
#define B1 1 
#define B1 1

#define B2 1  
#define B2 1

#define B3 1   
#define B3 1

//Define and redefine with different number of trailing
#define C12 1 
#define C12 1  

#define C23 1  
#define C23 1   

#define C34 1   
#define C34 1    

#define D21 1  
#define D21 1 

#define D32 1   
#define D32 1  

#define D43 1    
#define D43 1   

//Define without trailing tab and redefine with training tab
#define TAB_A1 1
#define TAB_A1 1	

//Define with trailing tab and redefine with no training tab
#define TAB_B1 1	
#define TAB_B1 1

//Define with trailing space and redefine with training tab
#define TAB_C1 1 
#define TAB_C1 1	

//Define with trailing tab and redefine with training space
#define TAB_D1 1	
#define TAB_D1 1 

/*
 * Definitions with no values
 */

//Define without trailing space and redefine with training spaces
#define NV_A1
#define NV_A1 

#define NV_A2
#define NV_A2  

#define NV_A3
#define NV_A3   

//Define with trailing space and redefine with no training space
#define NV_B1 
#define NV_B1

#define NV_B2  
#define NV_B2

#define NV_B3   
#define NV_B3

//Define and redefine with different number of trailing spaces
#define NV_C12 
#define NV_C12  

#define NV_C23  
#define NV_C23   

#define NV_C34   
#define NV_C34    

#define NV_D21  
#define NV_D21 

#define NV_D32   
#define NV_D32  

#define NV_D43    
#define NV_D43   

/* Some compilers generate spurious errors if a shader does not contain
 * any code or declarations.
 */
int foo(void) { return 1; }
