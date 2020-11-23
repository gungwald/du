/*
 * trace.h
 *
 *  Created on: Nov 14, 2020
 *      Author: bill
 */

#ifndef TRACE_H_
#define TRACE_H_

//#define TRACE
#ifdef TRACE
#define TRACE_ENTER(f,n,v)					_tprintf(_T("Enter %S %s=%s\n"),f,n,v)
#define TRACE_ENTER2(f,n,v,n2,v2)			_tprintf(_T("Enter %S %s=%s %s=%s\n"),f,n,v,n2,v2)
#define TRACE_ENTER_STR_UINT(f,n,v,n2,v2)	_tprintf(_T("Enter %S %s=%s %s=%u\n"),f,n,v,n2,v2)
#define TRACE_ENTER_CALLBACK(f,n,c,v)		_tprintf(_T("Enter %S %s="),f,n); c(v)
#define TRACE_RETURN(f,v)					_tprintf(_T("Return %S %s\n"),f,v)
#define TRACE_RETURN_ULONG(f,v)				_tprintf(_T("Return %S %lu\n"),f,v)
#define TRACE_RETURN_INT(f,v)				_tprintf(_T("Return %S %d\n"),f,v)
#else
#define TRACE_ENTER(f,n,v)
#define TRACE_ENTER2(f,n,v,n2,v2)
#define TRACE_ENTER_STR_UINT(f,n,v,n2,v2)
#define TRACE_ENTER_CALLBACK(f,n,c,v)
#define TRACE_RETURN(f,v)
#define TRACE_RETURN_ULONG(f,v)
#define TRACE_RETURN_INT(f,v)
#endif

#endif /* TRACE_H_ */
