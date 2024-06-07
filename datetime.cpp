/*************************************************
Copyright (C), 2020-2021, AIHua Tech. Co., Ltd.
File name:    datetime.cpp
Author:       yangchun
Version:      0.0.01
Date:         2020-11-30
Description:
History:
*************************************************/
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <climits>
#include <cstdio>
#include <cstring>
#include "datetime.h"

using namespace std;

/*
* We do not implement alternate representations. However, we always
* check whether a given modifier is allowed for a certain conversion.
*/
#define  ALT_E          0x01
#define  ALT_O          0x02
//#define LEGAL_ALT(x)       { if (alt_format & ~(x)) return (0); }
#define  LEGAL_ALT(x)       { ; }
#define  TM_YEAR_BASE   (1900)

static  int conv_num(const char **, int *, int, int);
static  int strncasecmp(char *s1, char *s2, size_t n);

static  const char *day[7] = {
	"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday",
	"Friday", "Saturday"
};
static  const char *abday[7] = {
	"Sun","Mon","Tue","Wed","Thu","Fri","Sat"
};
static  const char *mon[12] = {
	"January", "February", "March", "April", "May", "June", "July",
	"August", "September", "October", "November", "December"
};
static  const char *abmon[12] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};
static  const char *am_pm[2] = {
	"AM", "PM"
};

char* strptime(const char *buf, const char *fmt, struct tm *tm)
{
	char c;
	const char *bp;
	size_t len = 0;
	int alt_format, i, split_year = 0;

	bp = buf;

	while ((c = *fmt) != '\0') {
		/* Clear `alternate' modifier prior to new conversion. */
		alt_format = 0;

		/* Eat up white-space. */
		if (isspace(c)) {
			while (isspace(*bp))
				bp++;

			fmt++;
			continue;
		}

		if ((c = *fmt++) != '%')
			goto literal;


	again:        switch (c = *fmt++) {
	case '%': /* "%%" is converted to "%". */
		literal :
			if (c != *bp++)
				return (0);
		break;

		/*
		* "Alternative" modifiers. Just set the appropriate flag
		* and start over again.
		*/
	case 'E': /* "%E?" alternative conversion modifier. */
		LEGAL_ALT(0);
		alt_format |= ALT_E;
		goto again;

	case 'O': /* "%O?" alternative conversion modifier. */
		LEGAL_ALT(0);
		alt_format |= ALT_O;
		goto again;

		/*
		* "Complex" conversion rules, implemented through recursion.
		*/
	case 'c': /* Date and time, using the locale's format. */
		LEGAL_ALT(ALT_E);
		if (!(bp = strptime(bp, "%x %X", tm)))
			return (0);
		break;

	case 'D': /* The date as "%m/%d/%y". */
		LEGAL_ALT(0);
		if (!(bp = strptime(bp, "%m/%d/%y", tm)))
			return (0);
		break;

	case 'R': /* The time as "%H:%M". */
		LEGAL_ALT(0);
		if (!(bp = strptime(bp, "%H:%M", tm)))
			return (0);
		break;

	case 'r': /* The time in 12-hour clock representation. */
		LEGAL_ALT(0);
		if (!(bp = strptime(bp, "%I:%M:%S %p", tm)))
			return (0);
		break;

	case 'T': /* The time as "%H:%M:%S". */
		LEGAL_ALT(0);
		if (!(bp = strptime(bp, "%H:%M:%S", tm)))
			return (0);
		break;

	case 'X': /* The time, using the locale's format. */
		LEGAL_ALT(ALT_E);
		if (!(bp = strptime(bp, "%H:%M:%S", tm)))
			return (0);
		break;

	case 'x': /* The date, using the locale's format. */
		LEGAL_ALT(ALT_E);
		if (!(bp = strptime(bp, "%m/%d/%y", tm)))
			return (0);
		break;

		/*
		* "Elementary" conversion rules.
		*/
	case 'A': /* The day of week, using the locale's form. */
	case 'a':
		LEGAL_ALT(0);
		for (i = 0; i < 7; i++) {
			/* Full name. */
			len = strlen(day[i]);
			if (strncasecmp((char *)(day[i]), (char *)bp, len) == 0)
				break;

			/* Abbreviated name. */
			len = strlen(abday[i]);
			if (strncasecmp((char *)(abday[i]), (char *)bp, len) == 0)
				break;
		}

		/* Nothing matched. */
		if (i == 7)
			return (0);

		tm->tm_wday = i;
		bp += len;
		break;

	case 'B': /* The month, using the locale's form. */
	case 'b':
	case 'h':
		LEGAL_ALT(0);
		for (i = 0; i < 12; i++) {
			/* Full name. */
			len = strlen(mon[i]);
			if (strncasecmp((char *)(mon[i]), (char *)bp, len) == 0)
				break;

			/* Abbreviated name. */
			len = strlen(abmon[i]);
			if (strncasecmp((char *)(abmon[i]), (char *)bp, len) == 0)
				break;
		}

		/* Nothing matched. */
		if (i == 12)
			return (0);

		tm->tm_mon = i;
		bp += len;
		break;

	case 'C': /* The century number. */
		LEGAL_ALT(ALT_E);
		if (!(conv_num(&bp, &i, 0, 99)))
			return (0);

		if (split_year) {
			tm->tm_year = (tm->tm_year % 100) + (i * 100);
		}
		else {
			tm->tm_year = i * 100;
			split_year = 1;
		}
		break;

	case 'd': /* The day of month. */
	case 'e':
		LEGAL_ALT(ALT_O);
		if (!(conv_num(&bp, &tm->tm_mday, 1, 31)))
			return (0);
		break;

	case 'k': /* The hour (24-hour clock representation). */
		LEGAL_ALT(0);
		/* FALLTHROUGH */
	case 'H':
		LEGAL_ALT(ALT_O);
		if (!(conv_num(&bp, &tm->tm_hour, 0, 23)))
			return (0);
		break;

	case 'l': /* The hour (12-hour clock representation). */
		LEGAL_ALT(0);
		/* FALLTHROUGH */
	case 'I':
		LEGAL_ALT(ALT_O);
		if (!(conv_num(&bp, &tm->tm_hour, 1, 12)))
			return (0);
		if (tm->tm_hour == 12)
			tm->tm_hour = 0;
		break;

	case 'j': /* The day of year. */
		LEGAL_ALT(0);
		if (!(conv_num(&bp, &i, 1, 366)))
			return (0);
		tm->tm_yday = i - 1;
		break;

	case 'M': /* The minute. */
		LEGAL_ALT(ALT_O);
		if (!(conv_num(&bp, &tm->tm_min, 0, 59)))
			return (0);
		break;

	case 'm': /* The month. */
		LEGAL_ALT(ALT_O);
		if (!(conv_num(&bp, &i, 1, 12)))
			return (0);
		tm->tm_mon = i - 1;
		break;

	case 'S': /* The seconds. */
		LEGAL_ALT(ALT_O);
		if (!(conv_num(&bp, &tm->tm_sec, 0, 61)))
			return (0);
		break;

	case 'U': /* The week of year, beginning on sunday. */
	case 'W': /* The week of year, beginning on monday. */
		LEGAL_ALT(ALT_O);
		/*
		* XXX This is bogus, as we can not assume any valid
		* information present in the tm structure at this
		* point to calculate a real value, so just check the
		* range for now.
		*/
		if (!(conv_num(&bp, &i, 0, 53)))
			return (0);
		break;

	case 'w': /* The day of week, beginning on sunday. */
		LEGAL_ALT(ALT_O);
		if (!(conv_num(&bp, &tm->tm_wday, 0, 6)))
			return (0);
		break;

	case 'Y': /* The year. */
		LEGAL_ALT(ALT_E);
		if (!(conv_num(&bp, &i, 0, 9999)))
			return (0);

		tm->tm_year = i - TM_YEAR_BASE;
		break;

	case 'y': /* The year within 100 years of the epoch. */
		LEGAL_ALT(ALT_E | ALT_O);
		if (!(conv_num(&bp, &i, 0, 99)))
			return (0);

		if (split_year) {
			tm->tm_year = ((tm->tm_year / 100) * 100) + i;
			break;
		}
		split_year = 1;
		if (i <= 68)
			tm->tm_year = i + 2000 - TM_YEAR_BASE;
		else
			tm->tm_year = i + 1900 - TM_YEAR_BASE;
		break;

		/*
		* Miscellaneous conversions.
		*/
	case 'n': /* Any kind of white-space. */
	case 't':
		LEGAL_ALT(0);
		while (isspace(*bp))
			bp++;
		break;


	default: /* Unknown/unsupported conversion. */
		return (0);
	}


	}

	/* LINTED functional specification */
	return ((char *)bp);
}


static  int conv_num(const char **buf, int *dest, int llim, int ulim)
{
	int result = 0;

	/* The limit also determines the number of valid digits. */
	int rulim = ulim;

	if (**buf < '0' || **buf > '9')
		return (0);

	do {
		result *= 10;
		result += *(*buf)++ - '0';
		rulim /= 10;
	} while ((result * 10 <= ulim) && rulim && **buf >= '0' && **buf <= '9');

	if (result < llim || result > ulim)
		return (0);

	*dest = result;
	return (1);
}

int strncasecmp(char *s1, char *s2, size_t n)
{
	if (n == 0)
		return 0;

	while (n-- != 0 && tolower(*s1) == tolower(*s2))
	{
		if (n == 0 || *s1 == '/0' || *s2 == '/0')
			break;
		s1++;
		s2++;
	}

	return tolower(*(unsigned char *)s1) - tolower(*(unsigned char *)s2);
}

// 以当前时间设置对象
void DateTime::set() {
	_time = ::time(0);

	localtime_s(&_tm, &_time);
}

// 以 time_t 参数设置对象time_t类型参数
void DateTime::set(const time_t &tt) {
	time_t _tt = tt;

	if (tt < 0) _tt = 0;
	if (tt > LONG_MAX) _tt = LONG_MAX;

	_time = _tt;

	localtime_s(&_tm, &_time);
}

// 以指定时间设置对象
// 若参数不是有效日期时间,则设置为系统初始时间（1970/1/1）
// 若参数日期时间不存在,则设置为顺延有效时间（非闰年2/29视为3/1）
// \param year 年
// \param mon 月
// \param mday 日
// \param hour 时,默认为0
// \param min 分,默认为0
// \param src 秒,默认为0
void DateTime::set(const int year, const int mon, const int mday,
	const int hour, const int minute, const int sec)
{
	int _year = year;
	int _mon = mon;
	int _mday = mday;
	int _hour = hour;
	int _min = minute;
	int _sec = sec;

	// confirm
	if (_year<1)	_year = TIME_BASE_YEAR;
	if (_mon<1 || _mon>12) 		_mon = 1;
	if (_mday<1 || _mday>31)		_mday = 1;
	if (_hour<0 || _hour>23)		_hour = 0;
	if (_min<0 || _min>59) 		_min = 0;
	if (_sec<0 || _sec>59) 		_sec = 0;

	_tm.tm_year = _year - TIME_BASE_YEAR;
	_tm.tm_mon = _mon - 1;
	_tm.tm_mday = _mday;
	_tm.tm_hour = _hour;
	_tm.tm_min = _min;
	_tm.tm_sec = _sec;
	_tm.tm_isdst = -1;
	_time = mktime(&_tm);
}

/// 以 tm 结构参数设置对象
/// \param st struct tm类型参数
void DateTime::set(const tm &st) {
	this->set(st.tm_year + TIME_BASE_YEAR, st.tm_mon + 1, st.tm_mday,
		st.tm_hour, st.tm_min, st.tm_sec);
}

/// 以 DateTime 参数设置对象
/// \param date Date类型参数
void DateTime::set(const DateTime &date) {
	this->set(date.value());
}

/// 以"YYYY-MM-DD HH:MM:SS"格式字符串设置对象
/// 若字符串格式错误或者时间值错误则设置为当前时间
/// \param datetime "YYYY-MM-DD HH:MM:SS"格式日期时间字符串
/// \param datemark 日期分隔字符,默认为"-"
/// \param dtmark 日期时间分隔字符,默认为" ",不能与datemark或timemark相同
/// \param timemark 时间分隔字符,默认为":"
void DateTime::set(const string &datetime, const string &datemark,
	const string &dtmark, const string &timemark)
{
	// init struct tm
	struct tm tm;
	tm.tm_isdst = -1;

	// init format
	string fmt;
	if (datetime.find(dtmark) != datetime.npos)
		fmt = "%Y" + datemark + "%m" + datemark + "%d" + dtmark +
		"%H" + timemark + "%M" + timemark + "%S";
	else
		fmt = "%Y" + datemark + "%m" + datemark + "%d";

	// invoke strptime()
	if (strptime(datetime.c_str(), fmt.c_str(), &tm) != NULL)
		this->set(tm);
	else
		this->set();
}

/// 输出日期字符串
/// \param datemark 日期分隔字符,默认为"-"
/// \param leadingzero 是否补充前置零,默认为是
/// \return 输出指定格式的日期字符串
string DateTime::date(const string &datemark, const bool leadingzero) const {
	char date_str[32];
	if (leadingzero)
		snprintf(date_str, 32, "%04d%s%02d%s%02d",
			this->year(), datemark.c_str(), this->month(), datemark.c_str(), this->m_day());
	else
		snprintf(date_str, 32, "%d%s%d%s%d",
			this->year(), datemark.c_str(), this->month(), datemark.c_str(), this->m_day());

	return string(date_str);
}

/// 输出时间字符串
/// \param timemark 时间分隔字符,默认为":"
/// \param leadingzero 是否补充前置零,默认为是
/// \return 输出指定格式的时间字符串
string DateTime::time(const string &timemark, const bool leadingzero) const {
	char time_str[32];
	if (leadingzero)
		snprintf(time_str, 32, "%02d%s%02d%s%02d",
			this->hour(), timemark.c_str(), this->minute(), timemark.c_str(), this->sec());
	else
		snprintf(time_str, 32, "%d%s%d%s%d",
			this->hour(), timemark.c_str(), this->minute(), timemark.c_str(), this->sec());

	return string(time_str);
}

/// 输出日期时间字符串
/// \param datemark 日期分隔字符,默认为"-"
/// \param dtmark 日期时间分隔字符,默认为" "
/// \param timemark 时间分隔字符,默认为":"
/// \param leadingzero 是否补充前置零,默认为是
/// \return 输出指定格式的日期时间字符串
string DateTime::datetime(const string &datemark, const string &dtmark,
	const string &timemark, const bool leadingzero) const
{
	string datetime = this->date(datemark, leadingzero) + dtmark +
		this->time(timemark, leadingzero);
	return datetime;
}

/// 输出 GMT 格式日期时间字符串
/// 主要用于设置 cookie 有效期
/// \return GMT 格式日期时间字符串
string DateTime::gmt_datetime() const {
	char gmt[50];
	struct tm gmt_tm;

	gmtime_s(&gmt_tm, &_time);

	strftime(gmt, 50, "%A,%d-%B-%Y %H:%M:%S GMT", &gmt_tm);
	return string(gmt);
}

// 赋值操作
DateTime& DateTime::operator=(const DateTime &date) {
	if (this == &date) return *this;
	this->set(date);
	return *this;
}

// 赋值操作
DateTime& DateTime::operator=(const time_t &tt) {
	this->set(tt);
	return *this;
}

// 递增操作
DateTime& DateTime::operator+=(const DateTime &date) {
	this->set(value() + date.value());
	return *this;
}
// 递增操作
DateTime& DateTime::operator+=(const time_t &tt) {
	this->set(value() + tt);
	return *this;
}

// 递减操作
DateTime& DateTime::operator-=(const DateTime &date) {
	this->set(value() - date.value());
	return *this;
}

// 递减操作
DateTime& DateTime::operator-=(const time_t &tt) {
	this->set(value() - tt);
	return *this;
}

// 返回当月天数，范围1~31
int DateTime::m_days() const {
	int m = this->month();
	if (m == 1 || m == 3 || m == 5 || m == 7 || m == 8 || m == 10 || m == 12) {
		return 31;
	}
	else if (m == 2) {
		int leap = (this->year()) % 4;
		if (leap == 0) {
			return 29;
		}
		else {
			return 28;
		}
	}
	else {
		return 30;
	}
}

// 相加操作
DateTime operator+(const DateTime &date1, const DateTime &date2) {
	DateTime newdate;
	newdate.set(date1.value() + date2.value());
	return newdate;
}

//相加操作
DateTime operator+(const DateTime &date, const time_t &tt) {
	DateTime newdate;
	newdate.set(date.value() + tt);
	return newdate;
}

// 相减操作
DateTime operator-(const DateTime &date1, const DateTime &date2) {
	DateTime newdate;
	newdate.set(date1.value() - date2.value());
	return newdate;
}

// 相减操作
DateTime operator-(const DateTime &date, const time_t &tt) {
	DateTime newdate;
	newdate.set(date.value() - tt);
	return newdate;
}
