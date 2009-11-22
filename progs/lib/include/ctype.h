#ifndef	__CTYPE__H__
#define	__CTYPE__H__

#define	__isascii(c)	(((c) & ~0x7f) == 0)
#define	__toascii(c)	((c) & 0x7f)

#define isalnum(c) ( isdigit(c) || isalpha(c) )
#define isalpha(c) ( isupper(c) || islower(c) )
#define iscntrl(c) ( (c) < 32 )
#define isdigit(c) ( (c) >= '0' && (c) <= '9' )
#define islower(c) ( (c) >= 'a' && (c) <= 'z' )
#define isgraph(c) ( (c) > 32  && (c) < 126 )
#define isprint(c) ( isgraph(c) || c == ' ' ) 
#define ispunct(c) ( isgraph(c) && !isspace(c) && !isalnum(c) )
#define isspace(c) ( (c) == ' ' || (c) == '\t' || (c) == '\f' || (c) == '\n' || (c) == '\r' || (c) == '\v' )
#define isupper(c) ( (c) >= 'A' && (c) <= 'Z' )
#define isxdigit(c) ( isdigit(c) || ( (c) >= 'A' && (c) <= 'F' ) || ( (c) >= 'a' && (c) <= 'f' ) )

extern int tolower (int c);
extern int toupper (int c);

#endif // __CTYPE__H__
