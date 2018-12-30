/**
  * @author     Onur Efe
  */

#ifndef ASSERT_H
#define ASSERT_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef ASSERT
#define assert_param(cond) ((cond) ? (void)0 : Assert_Failed(__FILE__, __LINE__))
#else
#define assert_param(cond)
#endif

/* Includes ------------------------------------------------------------------*/

	/* Exported functions --------------------------------------------------------*/
	extern void Assert_Failed(const char *pFile, unsigned int line);

#ifdef __cplusplus
}
#endif

#endif
