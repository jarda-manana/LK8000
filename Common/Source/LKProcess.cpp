/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: LKProcess.cpp,v 1.8 2010/12/11 19:32:14 root Exp root $
*/

#include "StdAfx.h"
#include "options.h"
#include "Cpustats.h"
#include "XCSoar.h"
#include "Utils2.h"
#include "compatibility.h"
#include "MapWindow.h"
#include "Units.h"
#include "McReady.h"
#include "externs.h"
#include "InputEvents.h"
#include <windows.h>
#include <math.h>
#include <tchar.h>
#include "InfoBoxLayout.h"
#include "Logger.h"
#include "Process.h"
#include "Task.h"

// #define NULLSHORT	"--" 
#define NULLMEDIUM	"---"
#define NULLLONG	"---"
#define NULLTIME	"--:--"
#define INFINVAL	"oo"

extern int PDABatteryPercent;
extern int PDABatteryFlag;
extern int PDABatteryStatus;

// below this value, altitude differences are useless and not returned
//#define	ALTDIFFLIMIT	-2000


// Returns true if value is valid, false if not
// lktitle is shorter and limited to 6 or 7 chars, good for navboxes
// Units are empty by default, and valid is false by default
bool MapWindow::LKFormatValue(const short lkindex, const bool lktitle, TCHAR *BufferValue, TCHAR *BufferUnit, TCHAR *BufferTitle) {

  static int	index=-1;
  static double value;
  static int	ivalue;
  static char	text[LKSIZETEXT];
  static bool   doinit=true;
  static TCHAR	varformat[10];

  // By default, invalid return value. Set it to true after assigning value in cases
  bool		valid=false;

  if (doinit) {

	if (LIFTMODIFY==TOFEETPERMINUTE)
		_stprintf(varformat,TEXT("%%+.0f"));
	else
		_stprintf(varformat,TEXT("%%+0.1f"));

	doinit=false;
  }


	_tcscpy(BufferUnit,_T(""));

	switch(lkindex) {

		// B135
		case LK_TIME_LOCALSEC:
			Units::TimeToTextS(BufferValue, (int)DetectCurrentTime());
			valid=true;
			if (lktitle)
				// LKTOKEN  _@M1079_ = "Time local", _@M1080_ = "Time"
				_stprintf(BufferTitle, gettext(TEXT("_@M1080_")));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );

			break;
		// B39
		case LK_TIME_LOCAL:
			Units::TimeToText(BufferValue, (int)DetectCurrentTime());
			valid=true;
			if (lktitle)
				// LKTOKEN  _@M1079_ = "Time local", _@M1080_ = "Time"
				_stprintf(BufferTitle, gettext(TEXT("_@M1080_")));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );

			break;

		// B40
		case LK_TIME_UTC:
			Units::TimeToText(BufferValue,(int) GPS_INFO.Time);
			valid=true;
			if (lktitle)
				// LKTOKEN  _@M1081_ = "Time UTC", _@M1082_ = "UTC"
				_stprintf(BufferTitle, gettext(TEXT("_@M1082_")));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );

			break;

		// B03
		case LK_BRG:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				// LKTOKEN  _@M1007_ = "Bearing", _@M1008_ = "Brg"
				_stprintf(BufferTitle, gettext(TEXT("_@M1008_")));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			if ( ValidTaskPoint(ActiveWayPoint) != false ) {
				index = Task[ActiveWayPoint].Index;
				if (index>=0) {
					// This value in AAT is not the waypoint bearing!
					value = WayPointCalc[index].Bearing;
					valid=true;
#ifndef __MINGW32__
					if (value > 1)
						_stprintf(BufferValue, TEXT("%2.0f\xB0"), value);			//Source editor encoding problem fixed
					else if (value < -1)
						_stprintf(BufferValue, TEXT("%2.0f\xB0"), -value);
						else
							_tcscpy(BufferValue, TEXT("0\xB0"));
#else
					if (value > 1)
						_stprintf(BufferValue, TEXT("%2.0f°"), value);
					else if (value < -1)
						_stprintf(BufferValue, TEXT("%2.0f°"), -value);
						else
							_tcscpy(BufferValue, TEXT("0°"));
#endif
				}
			}
			break;

		// B78
		case LK_HOMERADIAL:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				// LKTOKEN  _@M1157_ = "Home Radial", _@M1158_ = "Radial"
				_stprintf(BufferTitle, gettext(TEXT("_@M1158_")));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );

			if ( ValidWayPoint(HomeWaypoint) != false ) {
				if (CALCULATED_INFO.HomeDistance >10.0) {
					// homeradial == 0, ok?
					value = CALCULATED_INFO.HomeRadial;
					valid=true;
#ifndef __MINGW32__
					if (value > 1)
						_stprintf(BufferValue, TEXT("%2.0f\xB0"), value);
					else if (value < -1)
						_stprintf(BufferValue, TEXT("%2.0f\xB0"), -value);
						else
							_tcscpy(BufferValue, TEXT("0\xB0"));
#else
					if (value > 1)
						_stprintf(BufferValue, TEXT("%2.0f°"), value);
					else if (value < -1)
						_stprintf(BufferValue, TEXT("%2.0f°"), -value);
						else
							_tcscpy(BufferValue, TEXT("0°"));
#endif
				}
			}
			break;

		// B47
		case LK_BRGDIFF:
			wsprintf(BufferValue,_T(NULLMEDIUM)); // 091221
			if (lktitle)
				// LKTOKEN  _@M1095_ = "Bearing Difference", _@M1096_ = "To"
				_stprintf(BufferTitle, gettext(TEXT("_@M1096_")));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			if ( ValidTaskPoint(ActiveWayPoint) != false ) {
				index = Task[ActiveWayPoint].Index;
				if (index>=0) {
#ifndef MAP_ZOOM
					if (DisplayMode != dmCircling)
#else /* MAP_ZOOM */
					if (!MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING))
#endif /* MAP_ZOOM */
					{
						value = WayPointCalc[index].Bearing -  GPS_INFO.TrackBearing;
						valid=true;
						if (value < -180.0)
							value += 360.0;
						else
							if (value > 180.0)
								value -= 360.0;
#ifndef __MINGW32__
						if (value > 1)
							_stprintf(BufferValue, TEXT("%2.0f\xB0\xBB"), value);
						else if (value < -1)
							_stprintf(BufferValue, TEXT("\xAB%2.0f\xB0"), -value);
							else
								_tcscpy(BufferValue, TEXT("\xAB\xBB"));
#else
						if (value > 1)
						_stprintf(BufferValue, TEXT("%2.0f°»"), value);
						else if (value < -1)
						_stprintf(BufferValue, TEXT("«%2.0f°"), -value);
						else
							_tcscpy(BufferValue, TEXT("«»"));
					}
#endif
				}
			}
			break;
#if 0
		// B151 UNUSED
		case LK_ALT1_BRGDIFF:
			wsprintf(BufferValue,_T(NULLMEDIUM)); 
			// LKTOKEN  _@M1095_ = "Bearing Difference", _@M1096_ = "To"
			_stprintf(BufferTitle, gettext(TEXT("_@M1096_")));
			if ( ValidWayPoint(Alternate1) != false ) {
				index = Alternate1;
				if (index>=0) {
					if (DisplayMode != dmCircling)
					{
						value = WayPointCalc[index].Bearing -  GPS_INFO.TrackBearing;
						valid=true;
						if (value < -180.0)
							value += 360.0;
						else
							if (value > 180.0)
								value -= 360.0;
#ifndef __MINGW32__
						if (value > 1)
							_stprintf(BufferValue, TEXT("%2.0f��"), value);
						else if (value < -1)
							_stprintf(BufferValue, TEXT("�%2.0f�"), -value);
							else
								_tcscpy(BufferValue, TEXT("��"));
#else
						if (value > 1)
						_stprintf(BufferValue, TEXT("%2.0f°»"), value);
						else if (value < -1)
						_stprintf(BufferValue, TEXT("«%2.0f°"), -value);
						else
							_tcscpy(BufferValue, TEXT("«»"));
					}
#endif
				}
			}
			break;

		// B152 UNUSED
		case LK_ALT2_BRGDIFF:
			wsprintf(BufferValue,_T(NULLMEDIUM)); 
			// LKTOKEN  _@M1095_ = "Bearing Difference", _@M1096_ = "To"
			_stprintf(BufferTitle, gettext(TEXT("_@M1096_")));
			if ( ValidWayPoint(Alternate2) != false ) {
				index = Alternate2;
				if (index>=0) {
					if (DisplayMode != dmCircling)
					{
						value = WayPointCalc[index].Bearing -  GPS_INFO.TrackBearing;
						valid=true;
						if (value < -180.0)
							value += 360.0;
						else
							if (value > 180.0)
								value -= 360.0;
#ifndef __MINGW32__
						if (value > 1)
							_stprintf(BufferValue, TEXT("%2.0f��"), value);
						else if (value < -1)
							_stprintf(BufferValue, TEXT("�%2.0f�"), -value);
							else
								_tcscpy(BufferValue, TEXT("��"));
#else
						if (value > 1)
						_stprintf(BufferValue, TEXT("%2.0f°»"), value);
						else if (value < -1)
						_stprintf(BufferValue, TEXT("«%2.0f°"), -value);
						else
							_tcscpy(BufferValue, TEXT("«»"));
					}
#endif
				}
			}
			break;

		// B153 UNUSED
		case LK_BALT_BRGDIFF:
			wsprintf(BufferValue,_T(NULLMEDIUM)); 
			// LKTOKEN  _@M1095_ = "Bearing Difference", _@M1096_ = "To"
			_stprintf(BufferTitle, gettext(TEXT("_@M1096_")));
			if ( ValidWayPoint(BestAlternate) != false ) {
				index = BestAlternate;
				if (index>=0) {
					if (DisplayMode != dmCircling)
					{
						value = WayPointCalc[index].Bearing -  GPS_INFO.TrackBearing;
						valid=true;
						if (value < -180.0)
							value += 360.0;
						else
							if (value > 180.0)
								value -= 360.0;
#ifndef __MINGW32__
						if (value > 1)
							_stprintf(BufferValue, TEXT("%2.0f��"), value);
						else if (value < -1)
							_stprintf(BufferValue, TEXT("�%2.0f�"), -value);
							else
								_tcscpy(BufferValue, TEXT("��"));
#else
						if (value > 1)
						_stprintf(BufferValue, TEXT("%2.0f°»"), value);
						else if (value < -1)
						_stprintf(BufferValue, TEXT("«%2.0f°"), -value);
						else
							_tcscpy(BufferValue, TEXT("«»"));
					}
#endif
				}
			}
			break;

		// B155 UNUSED
		case LK_LASTTHERMAL_BRGDIFF:
			wsprintf(BufferValue,_T(NULLMEDIUM)); 
			// LKTOKEN  _@M1095_ = "Bearing Difference", _@M1096_ = "To"
			_stprintf(BufferTitle, gettext(TEXT("_@M1096_")));
			if ( ValidResWayPoint(RESWP_LASTTHERMAL) != false ) {
				index = RESWP_LASTTHERMAL;
				if (index>=0) {
					if (DisplayMode != dmCircling)
					{
						value = WayPointCalc[index].Bearing -  GPS_INFO.TrackBearing;
						valid=true;
						if (value < -180.0)
							value += 360.0;
						else
							if (value > 180.0)
								value -= 360.0;
#ifndef __MINGW32__
						if (value > 1)
							_stprintf(BufferValue, TEXT("%2.0f��"), value);
						else if (value < -1)
							_stprintf(BufferValue, TEXT("�%2.0f�"), -value);
							else
								_tcscpy(BufferValue, TEXT("��"));
#else
						if (value > 1)
						_stprintf(BufferValue, TEXT("%2.0f°»"), value);
						else if (value < -1)
						_stprintf(BufferValue, TEXT("«%2.0f°"), -value);
						else
							_tcscpy(BufferValue, TEXT("«»"));
					}
#endif
				}
			}
			break;

#endif
		// B11
		case LK_NEXT_DIST:
			if ( ValidTaskPoint(ActiveWayPoint) != false ) {
				index = Task[ActiveWayPoint].Index;
				if (index>=0) {
					value=CALCULATED_INFO.WaypointDistance*DISTANCEMODIFY;
					valid=true;
					if (value>99)
						sprintf(text,"%.0f",value);
					else
						sprintf(text,"%.1f",value);
				} else {
					strcpy(text,NULLMEDIUM); // 091221
				}
			} else {
				strcpy(text,NULLMEDIUM); // 091221
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));
			if (lktitle)
				// LKTOKEN  _@M1023_ = "Next Distance", _@M1024_ = "Dist"
				_tcscpy(BufferTitle, gettext(TEXT("_@M1024_")));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;
#if 0
		// B148  UNUSED
		case LK_ALT1_DIST:
			if(ValidWayPoint(Alternate1)) {
				index = Alternate1;
				if (index>=0) {
					value=WayPointCalc[index].Distance*DISTANCEMODIFY;
					valid=true;
					if (value>99)
						sprintf(text,"%.0f",value);
					else
						sprintf(text,"%.1f",value);
				} else {
					strcpy(text,NULLMEDIUM);
				}
			} else {
				strcpy(text,NULLMEDIUM);
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));
			break;

		// B149  UNUSED
		case LK_ALT2_DIST:
			if(ValidWayPoint(Alternate2)) {
				index = Alternate2;
				if (index>=0) {
					value=WayPointCalc[index].Distance*DISTANCEMODIFY;
					valid=true;
					if (value>99)
						sprintf(text,"%.0f",value);
					else
						sprintf(text,"%.1f",value);
				} else {
					strcpy(text,NULLMEDIUM);
				}
			} else {
				strcpy(text,NULLMEDIUM);
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));
			break;

		// B150  100919 UNUSED
		case LK_BALT_DIST:
			if(ValidWayPoint(BestAlternate)) {
				index = BestAlternate;
				if (index>=0) {
					value=WayPointCalc[index].Distance*DISTANCEMODIFY;
					valid=true;
					if (value>99)
						sprintf(text,"%.0f",value);
					else
						sprintf(text,"%.1f",value);
				} else {
					strcpy(text,NULLMEDIUM);
				}
			} else {
				strcpy(text,NULLMEDIUM);
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));
			break;

		// B154  100919 UNUSED
		case LK_LASTTHERMAL_DIST:
			if(ValidResWayPoint(RESWP_LASTTHERMAL)) {
				index = RESWP_LASTTHERMAL;
				if (index>=0) {
					value=WayPointCalc[index].Distance*DISTANCEMODIFY;
					valid=true;
					if (value>99)
						sprintf(text,"%.0f",value);
					else
						sprintf(text,"%.1f",value);
				} else {
					strcpy(text,NULLMEDIUM);
				}
			} else {
				strcpy(text,NULLMEDIUM);
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));
			break;
#endif

		// B147 Distance from the start sector, always available also after start
		case LK_START_DIST:
			if ( ValidTaskPoint(0) && ValidTaskPoint(1) ) { // if real task
				index = Task[0].Index;
				if (index>=0) {
					value=(CALCULATED_INFO.WaypointDistance-StartRadius)*DISTANCEMODIFY;
					if (value<0) value*=-1; // 101112 BUGFIX
					valid=true;
					if (value>99)
						sprintf(text,"%.0f",value);
					else {
						if (value>10) {
							sprintf(text,"%.1f",value);
						} else 
							sprintf(text,"%.3f",value);
					}
				} else {
					strcpy(text,NULLMEDIUM); // 091221
				}
			} else {
				strcpy(text,NULLMEDIUM); // 091221
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));
			// LKTOKEN  _@M1192_ = "StDis"
			_tcscpy(BufferTitle, gettext(TEXT("_@M1192_")));
			break;


		// B60
		case LK_HOME_DIST:
			if (HomeWaypoint>=0) {
				if ( ValidWayPoint(HomeWaypoint) != false ) {
					value=CALCULATED_INFO.HomeDistance*DISTANCEMODIFY;
					valid=true;
					if (value>99)
						sprintf(text,"%.0f",value);
					else
						sprintf(text,"%.1f",value);
				} else {
					strcpy(text,NULLMEDIUM); // 091221
				}
			} else {
				strcpy(text,NULLMEDIUM); // 091221
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));
			if (lktitle)
				// LKTOKEN  _@M1121_ = "Home Distance", _@M1122_ = "HomeDis"
				wsprintf(BufferTitle, gettext(TEXT("_@M1122_")));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;

		// B83
		case LK_ODOMETER:
			if (CALCULATED_INFO.Odometer>0) {
				value=CALCULATED_INFO.Odometer*DISTANCEMODIFY;
				valid=true;
				if (value>99)
					sprintf(text,"%.0f",value);
				else
					sprintf(text,"%.1f",value);
			} else {
				strcpy(text,NULLMEDIUM); // 091221
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));
			if (lktitle)
				// LKTOKEN  _@M1167_ = "Odometer", _@M1168_ = "Odo"
				wsprintf(BufferTitle, gettext(TEXT("_@M1168_")));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;

		// B28
		case LK_AA_DISTMAX:
		// B29
		case LK_AA_DISTMIN:
			if ( (ValidTaskPoint(ActiveWayPoint) != false) && AATEnabled ) {
				index = Task[ActiveWayPoint].Index;
				if (index>=0) {
					if ( lkindex == LK_AA_DISTMAX )
						value = DISTANCEMODIFY*CALCULATED_INFO.AATMaxDistance ;
					else
						value = DISTANCEMODIFY*CALCULATED_INFO.AATMinDistance ;
					valid=true;
					if (value>99)
						sprintf(text,"%.0f",value);
					else
						sprintf(text,"%.1f",value);
				} else {
					strcpy(text,NULLMEDIUM); // 091221
				}
			} else {
				strcpy(text,NULLMEDIUM); // 091221
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));
			_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;

		// B51
		case LK_AA_TARG_DIST:
			if ( (ValidTaskPoint(ActiveWayPoint) != false) && AATEnabled ) {
				index = Task[ActiveWayPoint].Index;
				if (index>=0) {
					value = DISTANCEMODIFY*CALCULATED_INFO.AATTargetDistance ;
					valid=true;
					if (value>99)
						sprintf(text,"%.0f",value);
					else
						sprintf(text,"%.1f",value);
				} else {
					strcpy(text,NULLMEDIUM); // 091221
				}
			} else {
				strcpy(text,NULLMEDIUM); // 091221
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));
			_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;

		// B30
		case LK_AA_SPEEDMAX:
		// B31
		case LK_AA_SPEEDMIN:
			if ( (ValidTaskPoint(ActiveWayPoint) != false) && AATEnabled && CALCULATED_INFO.AATTimeToGo>=1 ) {
				index = Task[ActiveWayPoint].Index;
				if (index>=0) {
					if ( lkindex == LK_AA_SPEEDMAX )
						value = TASKSPEEDMODIFY*CALCULATED_INFO.AATMaxSpeed;
					else
						value = TASKSPEEDMODIFY*CALCULATED_INFO.AATMinSpeed;

					valid=true;
					sprintf(text,"%.0f",value);
				} else {
					strcpy(text,NULLMEDIUM);
				}
			} else {
				strcpy(text,NULLMEDIUM);
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetTaskSpeedName()));
			_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;

		// B52
		case LK_AA_TARG_SPEED:
			if ( (ValidTaskPoint(ActiveWayPoint) != false) && AATEnabled && CALCULATED_INFO.AATTimeToGo>=1 ) {
				index = Task[ActiveWayPoint].Index;
				if (index>=0) {
					value = TASKSPEEDMODIFY*CALCULATED_INFO.AATTargetSpeed;
					valid=true;
					sprintf(text,"%.0f",value);
				} else {
					strcpy(text,NULLMEDIUM);
				}
			} else {
				strcpy(text,NULLMEDIUM);
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetTaskSpeedName()));
			_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;

		// B72	WP REQ EFF
		case LK_NEXT_GR:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				// LKTOKEN  _@M1145_ = "Next Req.Efficiency", _@M1146_ = "Req.E"
				_stprintf(BufferTitle, gettext(TEXT("_@M1146_")));
			else
				// LKTOKEN  _@M1145_ = "Next Req.Efficiency", _@M1146_ = "Req.E"
				_stprintf(BufferTitle, gettext(TEXT("_@M1146_")));
			if ( ValidTaskPoint(ActiveWayPoint) != false ) {
				index = Task[ActiveWayPoint].Index;
				if (index>=0) {
					value=WayPointCalc[index].GR;
					if (value <1 || value >=ALTERNATE_MAXVALIDGR )
						strcpy(text,NULLMEDIUM);
					else {
						if (value >= 100) sprintf(text,"%.0lf",value);
							else sprintf(text,"%.1lf",value);
						valid=true;
					}
					wsprintf(BufferValue, TEXT("%S"),text);
				}
			}
			break;

		// B71	LD AVR 
		case LK_LD_AVR:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				// LKTOKEN  _@M1143_ = "Average Efficiency", _@M1144_ = "E.Avg"
				_stprintf(BufferTitle, gettext(TEXT("_@M1144_")));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
#ifndef MAP_ZOOM
			if (DisplayMode != dmCircling) {
#else /* MAP_ZOOM */
			if (!MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING)) {
#endif /* MAP_ZOOM */
				value=CALCULATED_INFO.AverageLD;
				if (value <1 ||  value >=ALTERNATE_MAXVALIDGR ) {
					strcpy(text,INFINVAL); 
					valid=true;
				} else
					if (value==0)
						sprintf(text,NULLMEDIUM);
					else {
						if (value<100)
							sprintf(text,"%.1f",value);
						else
							sprintf(text,"%2.0f",value);
						valid=true;
					}
				wsprintf(BufferValue, TEXT("%S"),text);
			}
			break;

		// B12
		// Arrival altitude using current MC  and total energy. Does not use safetymc.
		// total energy is disabled!
		case LK_NEXT_ALTDIFF:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				// LKTOKEN  _@M1025_ = "Next Alt.Arrival", _@M1026_ = "NxtArr"
				_stprintf(BufferTitle, gettext(TEXT("_@M1026_")));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			if ( ValidTaskPoint(ActiveWayPoint) != false ) {
				index = Task[ActiveWayPoint].Index;
				if (index>=0) {
					// don't use current MC...
					value=ALTITUDEMODIFY*WayPointCalc[index].AltArriv[AltArrivMode];
					if ( value > ALTDIFFLIMIT ) {
						valid=true;
						_stprintf(BufferValue,TEXT("%+1.0f"), value);
					}
				}
			}
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			break;

		// B13
		// Using MC! 
		case LK_NEXT_ALTREQ:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				// LKTOKEN  _@M1027_ = "Next Alt.Required", _@M1028_ = "NxtAltR"
				_stprintf(BufferTitle, gettext(TEXT("_@M1028_")));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			if ( ValidTaskPoint(ActiveWayPoint) != false ) {
				index = Task[ActiveWayPoint].Index;
				if (index>=0) {
					value=ALTITUDEMODIFY*WayPointCalc[index].AltReqd[AltArrivMode];
					if (value<10000 && value >-10000) {
						_stprintf(BufferValue,TEXT("%1.0f"), value);
						valid=true;
					} 
				}
			}
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			break;

		// B05
		case LK_LD_CRUISE:
			if (lktitle)
				// LKTOKEN  _@M1011_ = "Eff.cruise last therm", _@M1012_ = "E.Cru"
				_stprintf(BufferTitle, gettext(TEXT("_@M1012_")));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			value=DerivedDrawInfo.CruiseLD;
			if (value <-99 ||  value >=ALTERNATE_MAXVALIDGR ) {
				strcpy(text,INFINVAL); 
				valid=true;
			} else
			if (value==0) sprintf(text,NULLMEDIUM);
			else {
				sprintf(text,"%.0f",value);
				valid=true;
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			break;

		// B04
		case LK_LD_INST:
                        wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				// LKTOKEN  _@M1009_ = "Eff.last 20 sec", _@M1010_ = "E.20\""
				_stprintf(BufferTitle, gettext(TEXT("_@M1010_")));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			value=CALCULATED_INFO.LD;
			if (value <-99 ||  value >=ALTERNATE_MAXVALIDGR ) {
				strcpy(text,INFINVAL);
				valid=true;
			} else
				if (value==0) sprintf(text,NULLMEDIUM);
				else {
					sprintf(text,"%.0f",value);
					valid=true;
				}
			wsprintf(BufferValue, TEXT("%S"),text);
			break;

		// B00
		case LK_HNAV:
			if (lktitle)
				// LKTOKEN  _@M1001_ = "Altitude QNH", _@M1002_ = "Alt", 
				_stprintf(BufferTitle, gettext(TEXT("_@M1002_")));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			value=ALTITUDEMODIFY*DerivedDrawInfo.NavAltitude;
			valid=true;
			sprintf(text,"%d",(int)value);
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			break;

		// B01		AltAgl HAGL 100318
		case LK_HAGL:
			if (lktitle)
				// LKTOKEN  _@M1003_ = "Altitude AGL", _@M1004_ = "HAGL"
				_stprintf(BufferTitle, gettext(TEXT("_@M1004_")));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );

			if (!CALCULATED_INFO.TerrainValid) { 
				wsprintf(BufferValue, TEXT(NULLLONG));
				wsprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
				valid=false;
				break;
			}
			value=ALTITUDEMODIFY*DerivedDrawInfo.AltitudeAGL;
			valid=true;
			sprintf(text,"%d",(int)value);
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			break;

		// B33
		case LK_HBARO:
			if (GPS_INFO.BaroAltitudeAvailable) {
				value=ALTITUDEMODIFY*DrawInfo.BaroAltitude;
				valid=true;
				sprintf(text,"%d",(int)value);
				wsprintf(BufferValue, TEXT("%S"),text);
			} else
				wsprintf(BufferValue, TEXT(NULLLONG));
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;

		// B86
		case LK_HGPS:
			if (lktitle)
				// LKTOKEN  _@M1173_ = "Altitude GPS", _@M1174_ = "HGPS"
				_stprintf(BufferTitle, gettext(TEXT("_@M1174_")));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			if (GPS_INFO.NAVWarning || (GPS_INFO.SatellitesUsed == 0)) {
				wsprintf(BufferValue, TEXT(NULLLONG));
				valid=false;
			} else {
				value=ALTITUDEMODIFY*GPS_INFO.Altitude;
				valid=true;
				sprintf(text,"%d",(int)value);
				wsprintf(BufferValue, TEXT("%S"),text);
				wsprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			}
			break;

		// B70
		case LK_QFE:
			value=ALTITUDEMODIFY*DerivedDrawInfo.NavAltitude-QFEAltitudeOffset;;
			valid=true;
			sprintf(text,"%d",(int)value);
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;

		// B20
		case LK_HGND:
                        wsprintf(BufferValue,_T(NULLLONG));
			if (DerivedDrawInfo.TerrainValid) {
				value=ALTITUDEMODIFY*DerivedDrawInfo.TerrainAlt;
				valid=true;
				sprintf(text,"%d",(int)value);
				wsprintf(BufferValue, TEXT("%S"),text);
			}
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			if (lktitle)
				// LKTOKEN  _@M1041_ = "Terrain Elevation", _@M1042_ = "Gnd"
				wsprintf(BufferTitle, gettext(TEXT("_@M1042_")));
			else 
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;

		// B23
		case LK_TRACK:
			wsprintf(BufferValue,_T(NULLLONG));
			//_stprintf(BufferUnit,TEXT(""));
			if (lktitle)
				// LKTOKEN  _@M1047_ = "Track", _@M1048_ = "Track"
				_stprintf(BufferTitle, gettext(TEXT("_@M1048_")));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			value = GPS_INFO.TrackBearing;
			valid=true;
#ifndef __MINGW32__
			if (value > 1)
				_stprintf(BufferValue, TEXT("%2.0f\xB0"), value);
			else if (value < -1)
				_stprintf(BufferValue, TEXT("%2.0f\xB0"), -value);
				else
					_tcscpy(BufferValue, TEXT("0\xB0"));
#else
			if (value > 1)
				_stprintf(BufferValue, TEXT("%2.0f°"), value);
			else if (value < -1)
				_stprintf(BufferValue, TEXT("%2.0f°"), -value);
				else
					_tcscpy(BufferValue, TEXT("0°"));
#endif
			break;


		// B06
		case LK_GNDSPEED:
			value=SPEEDMODIFY*DrawInfo.Speed;
			valid=true;
			if (value<0||value>9999) value=0; else valid=true;
			sprintf(text,"%d",(int)value);
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetHorizontalSpeedName()));
			if (lktitle)
				// LKTOKEN  _@M1013_ = "Speed ground", _@M1014_ = "GS"
				wsprintf(BufferTitle, gettext(TEXT("_@M1014_")));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;


		// B32
		case LK_IAS:
			if (GPS_INFO.AirspeedAvailable) {
				if (lktitle)
					// LKTOKEN  _@M1065_ = "Airspeed IAS", _@M1066_ = "IAS"
					wsprintf(BufferTitle, gettext(TEXT("_@M1066_")));
				else
					_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
				value=SPEEDMODIFY*DrawInfo.IndicatedAirspeed;
				if (value<0||value>999) value=0; else valid=true;
				sprintf(text,"%d",(int)value);
				wsprintf(BufferValue, TEXT("%S"),text);
			} else {
				// LKTOKEN  _@M1065_ = "Airspeed IAS", _@M1066_ = "IAS"
				wsprintf(BufferTitle, TEXT("e%s"), gettext(TEXT("_@M1066_")));
				value=SPEEDMODIFY*DerivedDrawInfo.IndicatedAirspeedEstimated;
				if (value<0||value>999) value=0; else valid=true;
				sprintf(text,"%d",(int)value);
				wsprintf(BufferValue, TEXT("%S"),text);
			}
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetHorizontalSpeedName()));
			break;
				
		// B43 AKA STF
		case LK_SPEED_DOLPHIN:
			// if (GPS_INFO.AirspeedAvailable) {
				value=SPEEDMODIFY*DerivedDrawInfo.VOpt;
				if (value<0||value>999) value=0; else valid=true;
				sprintf(text,"%d",(int)value);
				wsprintf(BufferValue, TEXT("%S"),text);
			// } else
			//	wsprintf(BufferValue, TEXT(NULLMEDIUM));
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetHorizontalSpeedName()));
			_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;

		// B87  100908
		case LK_EQMC:
			// LKTOKEN  _@M1175_ = "MacCready Equivalent", _@M1176_ = "eqMC"
			wsprintf(BufferTitle, gettext(TEXT("_@M1176_")));
			if ( CALCULATED_INFO.Circling == TRUE || CALCULATED_INFO.EqMc<0 || CALCULATED_INFO.OnGround == TRUE) {
				wsprintf(BufferValue, TEXT(NULLMEDIUM));
			} else {
				value = iround(LIFTMODIFY*CALCULATED_INFO.EqMc*10)/10.0;
				valid=true;
				sprintf(text,"%2.1lf",value);
				wsprintf(BufferValue, TEXT("%S"),text);
			}
			break;
				
		// B44
		case LK_NETTO:
			value=LIFTMODIFY*DerivedDrawInfo.NettoVario;
			if (value<-100||value>100) value=0; else valid=true;
			_stprintf(BufferValue,varformat,value);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetVerticalSpeedName()));
			_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;


		// B54 091221
		case LK_TAS:
			if (GPS_INFO.AirspeedAvailable) {
				if (lktitle)
					// LKTOKEN  _@M1109_ = "Airspeed TAS", _@M1110_ = "TAS"
					wsprintf(BufferTitle, gettext(TEXT("_@M1110_")));
				else
					_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
				value=SPEEDMODIFY*DrawInfo.TrueAirspeed;
				if (value<0||value>999) {
					sprintf(text,"%s",NULLMEDIUM);
				} else {
					valid=true;
					sprintf(text,"%d",(int)value);
				}
				wsprintf(BufferValue, TEXT("%S"),text);
			} else {
				// LKTOKEN  _@M1109_ = "Airspeed TAS", _@M1110_ = "TAS"
				wsprintf(BufferTitle, TEXT("e%s"), gettext(TEXT("_@M1110_")));
				value=SPEEDMODIFY*DerivedDrawInfo.TrueAirspeedEstimated;
				if (value<0||value>999) {
					sprintf(text,"%s",NULLMEDIUM);
				} else {
					valid=true;
					sprintf(text,"%d",(int)value);
				}
				wsprintf(BufferValue, TEXT("%S"),text);
			}
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetHorizontalSpeedName()));
			break;

		// B55  Team Code 091216
		case LK_TEAM_CODE:
			if(ValidWayPoint(TeamCodeRefWaypoint)) {
				_tcsncpy(BufferValue,CALCULATED_INFO.OwnTeamCode,5);
				BufferValue[5] = '\0';
				valid=true; // 091221
			} else
				wsprintf(BufferValue,_T("----"));
			if (lktitle)
				// LKTOKEN  _@M1111_ = "Team Code", _@M1112_ = "TeamCode"
				wsprintf(BufferTitle, gettext(TEXT("_@M1112_")));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;

		// B56  Team Code 091216
		case LK_TEAM_BRG:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				// LKTOKEN  _@M1113_ = "Team Bearing", _@M1114_ = "TmBrng"
				_stprintf(BufferTitle, gettext(TEXT("_@M1114_")));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );

			if(ValidWayPoint(TeamCodeRefWaypoint) && TeammateCodeValid) {
				value=CALCULATED_INFO.TeammateBearing;
				valid=true;
				if (value > 1)
					_stprintf(BufferValue, TEXT("%2.0f°"), value);
				else if (value < -1)
					_stprintf(BufferValue, TEXT("%2.0f°"), -value);
				else
					_tcscpy(BufferValue, TEXT("0°"));
			}
			break;

		// B57 Team Bearing Difference 091216
		case LK_TEAM_BRGDIFF:
			wsprintf(BufferValue,_T(NULLLONG));

			if (lktitle)
				// LKTOKEN  _@M1115_ = "Team Bearing Diff", _@M1116_ = "TeamBd"
				_stprintf(BufferTitle, gettext(TEXT("_@M1116_")));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );

			if (ValidWayPoint(TeamCodeRefWaypoint) && TeammateCodeValid) {
				value = CALCULATED_INFO.TeammateBearing -  GPS_INFO.TrackBearing;
				valid=true; // 091221
				if (value < -180.0)
					value += 360.0;
				else
					if (value > 180.0) value -= 360.0;

				if (value > 1)
					_stprintf(BufferValue, TEXT("%2.0f°»"), value);
				else if (value < -1)
					_stprintf(BufferValue, TEXT("«%2.0f°"), -value);
					else
						_tcscpy(BufferValue, TEXT("«»"));
			}
			break;

		// B58 091216 Team Range Distance
		case LK_TEAM_DIST:
			if ( TeammateCodeValid ) {
				value=DISTANCEMODIFY*CALCULATED_INFO.TeammateRange;
				valid=true;
				if (value>99)
					sprintf(text,"%.0f",value);
				else
					sprintf(text,"%.1f",value);
			} else {
				strcpy(text,NULLLONG);
			}

			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));
			if (lktitle)
				// LKTOKEN  _@M1117_ = "Team Range", _@M1118_ = "TeamDis"
				_tcscpy(BufferTitle, gettext(TEXT("_@M1118_")));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;

		// B34
		case LK_SPEED_MC:
			value=SPEEDMODIFY*DerivedDrawInfo.VMacCready;
			valid=true;
			if (value<=0||value>999) value=0; else valid=true;
			sprintf(text,"%d",(int)value);
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetHorizontalSpeedName()));
			if (lktitle)
				// LKTOKEN  _@M1069_ = "Speed MacReady", _@M1070_ = "SpMc"
				wsprintf(BufferTitle, gettext(TEXT("_@M1070_")));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;


		// B35
		case LK_PRCCLIMB:
			value=DerivedDrawInfo.PercentCircling;
			valid=true;
			sprintf(text,"%d",(int)value);
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%%"));
			_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;


		// B73
		case LK_FL:
			if (lktitle)
				// LKTOKEN  _@M1147_ = "Flight Level", _@M1148_ = "FL"
				_stprintf(BufferTitle, gettext(TEXT("_@M1148_")));
			else
				// LKTOKEN  _@M1147_ = "Flight Level", _@M1148_ = "FL"
				_stprintf(BufferTitle, gettext(TEXT("_@M1148_")));
			value=(TOFEET*DerivedDrawInfo.NavAltitude)/100.0;
			valid=true;
			sprintf(text,"%d",(int)value);
			wsprintf(BufferValue, TEXT("%S"),text);
			//_stprintf(BufferUnit,TEXT(""));
			break;

		// B131
		case LK_WIND:
			// LKTOKEN  _@M1185_ = "Wind"
			_stprintf(BufferTitle, gettext(TEXT("_@M1185_")));
			if (DerivedDrawInfo.WindSpeed*SPEEDMODIFY>=1) {
				value = DerivedDrawInfo.WindBearing;
				valid=true;
				if (value==360) value=0;
				if (HideUnits)
					_stprintf(BufferValue,TEXT("%1.0f/%1.0f"), 
						value, SPEEDMODIFY*DerivedDrawInfo.WindSpeed );
				else
					_stprintf(BufferValue,TEXT("%1.0f")_T(DEG)_T("/%1.0f"), 
						value, SPEEDMODIFY*DerivedDrawInfo.WindSpeed );
			} else {
				_stprintf(BufferValue,TEXT("--/--"));
			}
			break;

		// B25
		case LK_WIND_SPEED:
			_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			wsprintf(BufferUnit, TEXT("%s"),Units::GetHorizontalSpeedName());
			
			value=DerivedDrawInfo.WindSpeed*SPEEDMODIFY;
			if (value>=1 ) {
				_stprintf(BufferValue,TEXT("%1.0f"), value );
				valid=true;
			} else {
				_stprintf(BufferValue,TEXT(NULLMEDIUM)); // 091221
			}
			break;

		// B26
		case LK_WIND_BRG:
			_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			if (DerivedDrawInfo.WindSpeed*SPEEDMODIFY>=1) {
				value = DerivedDrawInfo.WindBearing;
				valid=true;
				if (value==360) value=0;
				_stprintf(BufferValue,TEXT("%1.0f")_T(DEG), value );
			} else {
				_stprintf(BufferValue,TEXT(NULLMEDIUM));
			}
			
			break;

		// B07  091221
		case LK_TL_AVG:
			value= LIFTMODIFY*CALCULATED_INFO.LastThermalAverage;
			if (value==0)
				sprintf(text,NULLMEDIUM);
			else { 
				valid=true;
				if (value<20) sprintf(text,"%+.1lf",value);
					else sprintf(text,"%+.0lf",value);
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),Units::GetVerticalSpeedName());
			if (lktitle)
				// LKTOKEN  _@M1015_ = "Thermal Average Last", _@M1016_ = "TL.Avg"
				wsprintf(BufferTitle, gettext(TEXT("_@M1016_")));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;

		// B08 091216 091221
		case LK_TL_GAIN:
			value=ALTITUDEMODIFY*DerivedDrawInfo.LastThermalGain;
			if (value==0)
				sprintf(text,NULLMEDIUM);
			else { 
				valid=true;
				sprintf(text,"%+d",(int)value);
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			if (lktitle)
				// LKTOKEN  _@M1017_ = "Thermal Gain Last", _@M1018_ = "TL.Gain"
				wsprintf(BufferTitle, gettext(TEXT("_@M1018_")));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;

		// B21 091221
		case LK_TC_AVG:
			value= LIFTMODIFY*CALCULATED_INFO.AverageThermal;
			if (value==0)
				sprintf(text,NULLMEDIUM);
			else { 
				if (value<20) sprintf(text,"%+.1lf",value);
				else sprintf(text,"%+.0lf",value);
				valid=true; 
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),Units::GetVerticalSpeedName());
			if (lktitle)
				// LKTOKEN  _@M1043_ = "Thermal Average", _@M1044_ = "TC.Avg"
				wsprintf(BufferTitle, gettext(TEXT("_@M1044_")));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;

		// B10
		case LK_MC:
			value = iround(LIFTMODIFY*MACCREADY*10)/10.0;
			valid=true;
			//sprintf(text,"%.1lf",value);
			sprintf(text,"%2.1lf",value);
			wsprintf(BufferValue, TEXT("%S"),text);
			//if (!ValidTaskPoint(ActiveWayPoint) && ((AutoMcMode==0) || (AutoMcMode==2))) {
			if (!CALCULATED_INFO.AutoMacCready) {
				if (lktitle)
					// LKTOKEN  _@M1183_ = "ManMC"
					wsprintf(BufferTitle, gettext(TEXT("_@M1183_")));
				else
					_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			} else {
				if (lktitle)
					// LKTOKEN  _@M1184_ = "AutMC"
					wsprintf(BufferTitle, gettext(TEXT("_@M1184_")));
				else
					_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			}

			break;

		// B2 091221
		case LK_TC_30S:
			value=LIFTMODIFY*DerivedDrawInfo.Average30s;
			if (value==0)
				sprintf(text,NULLMEDIUM);
			else { 
				valid=true;
				if (value<20) sprintf(text,"%+.1lf",value);
					else sprintf(text,"%+.0lf",value);
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),Units::GetVerticalSpeedName());
			if (lktitle)
				// LKTOKEN  _@M1005_ = "Thermal last 30 sec", _@M1006_ = "TC.30\""
				wsprintf(BufferTitle, gettext(TEXT("_@M1006_")));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;

		// B22 091221
		case LK_TC_GAIN:
			value=ALTITUDEMODIFY*DerivedDrawInfo.ThermalGain;
			if (value==0)
				sprintf(text,NULLMEDIUM);
			else { 
				valid=true;
				sprintf(text,"%+d",(int)value);
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			if (lktitle)
				// LKTOKEN  _@M1045_ = "Thermal Gain", _@M1046_ = "TC.Gain"
				wsprintf(BufferTitle, gettext(TEXT("_@M1046_")));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;

		// B63 091221
		case LK_TC_ALL:
			if (CALCULATED_INFO.timeCircling <=0)
				//value=0.0;
				sprintf(text,NULLMEDIUM);
			else {
				value = LIFTMODIFY*CALCULATED_INFO.TotalHeightClimb /CALCULATED_INFO.timeCircling;
				if (value<20)
					sprintf(text,"%+.1lf",value);
				else
					sprintf(text,"%+.0lf",value);
				valid=true;
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),Units::GetVerticalSpeedName());
			if (lktitle)
				// LKTOKEN  _@M1127_ = "Thermal All", _@M1128_ = "Th.All"
				wsprintf(BufferTitle, gettext(TEXT("_@M1128_")));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;

		// B24
		case LK_VARIO:
			if (GPS_INFO.VarioAvailable) {
				value = LIFTMODIFY*GPS_INFO.Vario;
			} else {
				value = LIFTMODIFY*CALCULATED_INFO.Vario;
			}
			valid=true;
			_stprintf(BufferValue,varformat,value);
			wsprintf(BufferUnit, TEXT("%s"),Units::GetVerticalSpeedName());
			if (lktitle)
				// LKTOKEN  _@M1049_ = "Vario", _@M1050_ = "Vario"
				wsprintf(BufferTitle, gettext(TEXT("_@M1050_")));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;

		// B15 Arrival altitude , no more total energy
		case LK_FIN_ALTDIFF:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				// LKTOKEN  _@M1031_ = "Task Alt.Arrival", _@M1032_ = "TskArr"
				_stprintf(BufferTitle, gettext(TEXT("_@M1032_")));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			if ( ValidTaskPoint(ActiveWayPoint) != false ) {
				index = Task[ActiveWayPoint].Index;
				if (index>=0) {
					value=ALTITUDEMODIFY*DerivedDrawInfo.TaskAltitudeDifference;
					if ( value > ALTDIFFLIMIT ) {
						valid=true;
						_stprintf(BufferValue,TEXT("%+1.0f"), value);
					}
				}
			}
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			break;

		// B16
		case LK_FIN_ALTREQ:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				// LKTOKEN  _@M1033_ = "Task Alt.Required", _@M1034_ = "TskAltR"
				_stprintf(BufferTitle, gettext(TEXT("_@M1034_")));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			if ( ValidTaskPoint(ActiveWayPoint) != false ) {
				index = Task[ActiveWayPoint].Index;
				if (index>=0) {
					value=ALTITUDEMODIFY*DerivedDrawInfo.TaskAltitudeRequired;
					if (value<10000 && value >-10000) {
						_stprintf(BufferValue,TEXT("%1.0f"), value);
						valid=true;
					} 
				}
			}
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			break;

		// B18
		case LK_FIN_DIST:
			if ( ValidTaskPoint(ActiveWayPoint) != false ) {
				index = Task[ActiveWayPoint].Index;
				if (index>=0) {
					if (CALCULATED_INFO.ValidFinish) {
						value = DISTANCEMODIFY*CALCULATED_INFO.WaypointDistance;
					} else {
						value = DISTANCEMODIFY*CALCULATED_INFO.TaskDistanceToGo;
					}
					valid=true;
					if (value>99)
						sprintf(text,"%.0f",value);
					else
						sprintf(text,"%.1f",value);
				} else {
					strcpy(text,NULLLONG);
				}
			} else {
				strcpy(text,NULLLONG);
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));
			if (lktitle)
				// LKTOKEN  _@M1037_ = "Task Distance", _@M1038_ = "TskDis"
				_tcscpy(BufferTitle, gettext(TEXT("_@M1038_")));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;

		// B74
		case LK_TASK_DISTCOV:
			if ( (ActiveWayPoint >=1) && ( ValidTaskPoint(ActiveWayPoint) )) {
				value = DISTANCEMODIFY*CALCULATED_INFO.TaskDistanceCovered;
				valid=true;
				sprintf(text,"%.0f",value); // l o f?? TODO CHECK
			} else {
				strcpy(text,NULLLONG);
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));
			if (lktitle)
				// LKTOKEN  _@M1149_ = "Task Covered distance", _@M1150_ = "TskCov"
				wsprintf(BufferTitle, gettext(TEXT("_@M1150_")));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;

		// B79
		case LK_AIRSPACEDIST:
			if (lktitle)
				// LKTOKEN  _@M1159_ = "Airspace Distance", _@M1160_ = "AirSpace"
				wsprintf(BufferTitle, gettext(TEXT("_@M1160_")));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );

			if (NearestAirspaceHDist >0) {
				value = DISTANCEMODIFY*NearestAirspaceHDist;
				sprintf(text,"%1.1f",value);
				wsprintf(BufferValue, TEXT("%S"),text);
				valid = true;
			} else {
				valid=false;
				wsprintf(BufferValue, TEXT(NULLMEDIUM),text);
				value = -1;
			}
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));
			break;

		// B66
		case LK_FIN_GR:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				// LKTOKEN  _@M1133_ = "Task Req.Efficiency", _@M1134_ = "TskReqE"
				_stprintf(BufferTitle, gettext(TEXT("_@M1134_")));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			if ( ValidTaskPoint(ActiveWayPoint) != false ) {
				index = Task[ActiveWayPoint].Index;
				if (index>=0) {
					// the ValidFinish() seem to return FALSE when is actually valid.
					// In any case we do not use it for the vanilla GR
					value = CALCULATED_INFO.GRFinish;
					if (value <1 || value >=ALTERNATE_MAXVALIDGR )
						strcpy(text,NULLMEDIUM);
					else {
						if (value >= 100) sprintf(text,"%.0lf",value);
							else sprintf(text,"%.1lf",value);
						valid=true;
					}
					wsprintf(BufferValue, TEXT("%S"),text);
				}
			}
			break;


		// B19
		case LK_FIN_LD:
			wsprintf(BufferValue,_T(NULLLONG));
			//_stprintf(BufferUnit,TEXT(""));
			if (lktitle)
				// LKTOKEN  _@M1039_ = "_Reserved 1", _@M1040_ = "OLD fLD"
				_stprintf(BufferTitle, gettext(TEXT("_@M1040_")));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			if ( ValidTaskPoint(ActiveWayPoint) != false ) {
				index = Task[ActiveWayPoint].Index;
				if (index>=0) {
					if (CALCULATED_INFO.ValidFinish) {
						value = 0;
					} else {
						value = CALCULATED_INFO.LDFinish;
					}
					if (value <1 || value >=ALTERNATE_MAXVALIDGR )
						strcpy(text,NULLMEDIUM);
					else {
						valid=true;
						if (value >= 100) sprintf(text,"%.0lf",value);
							else sprintf(text,"%.1lf",value);
					}
					wsprintf(BufferValue, TEXT("%S"),text);
				}
			}
			break;

		// B38
		case LK_NEXT_LD:
			wsprintf(BufferValue,_T(NULLMEDIUM)); // 091221
			if (lktitle)
				// LKTOKEN  _@M1077_ = "_Reserved 2", _@M1078_ = "OLD nLD"
				_stprintf(BufferTitle, gettext(TEXT("_@M1078_")));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			if ( ValidTaskPoint(ActiveWayPoint) != false ) {
				index = Task[ActiveWayPoint].Index;
				if (index>=0) {
					value = CALCULATED_INFO.LDNext;
					if (value <1 || value >=ALTERNATE_MAXVALIDGR )
						strcpy(text,NULLMEDIUM);
					else {
						valid=true;
						if (value >= 100) sprintf(text,"%.0lf",value);
							else sprintf(text,"%.1lf",value);
					}
					wsprintf(BufferValue, TEXT("%S"),text);
				}
			}
			break;

		// B53
		case LK_LD_VARIO:
			wsprintf(BufferValue,_T(NULLMEDIUM));
			//_stprintf(BufferUnit,TEXT(""));
			_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			if (GPS_INFO.AirspeedAvailable && GPS_INFO.VarioAvailable) {
				value = CALCULATED_INFO.LDvario;
				if (value <1 || value >=ALTERNATE_MAXVALIDGR )
					strcpy(text,NULLMEDIUM);
				else {
					valid=true;
					if (value >= 100) sprintf(text,"%.0lf",value);
						else sprintf(text,"%.1lf",value);
				}
				wsprintf(BufferValue, TEXT("%S"),text);
			}
			break;

		// B64
		case LK_VARIO_DIST:
			wsprintf(BufferValue,_T(NULLMEDIUM));
			if ( ActiveWayPoint >=1) {
				if ( ValidTaskPoint(ActiveWayPoint) ) {
					value = LIFTMODIFY*CALCULATED_INFO.DistanceVario;
					_stprintf(BufferValue,varformat,value);
					valid=true;
				}
			}
			_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			wsprintf(BufferUnit, TEXT("%s"),Units::GetVerticalSpeedName());
			break;


		// B59
		case LK_SPEEDTASK_INST:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				// LKTOKEN  _@M1119_ = "Task Speed Instantaneous", _@M1120_ = "TskSpI"
				_stprintf(BufferTitle, gettext(TEXT("_@M1120_")));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			value=0;
			if ( ActiveWayPoint >=1) {
				if ( ValidTaskPoint(ActiveWayPoint) ) {
					value = TASKSPEEDMODIFY*CALCULATED_INFO.TaskSpeedInstantaneous;
					if (value<=0||value>999) value=0; else valid=true;
					sprintf(text,"%d",(int)value);
					wsprintf(BufferValue, TEXT("%S"),text);
				}
			}
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetTaskSpeedName()));
			break;

		// B61
		case LK_SPEEDTASK_ACH:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				// LKTOKEN  _@M1123_ = "Task Speed", _@M1124_ = "TskSp"
				_stprintf(BufferTitle, gettext(TEXT("_@M1124_")));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			value=0;
			if ( ActiveWayPoint >=1) {
				if ( ValidTaskPoint(ActiveWayPoint) ) {
					value = TASKSPEEDMODIFY*CALCULATED_INFO.TaskSpeedAchieved;
					if (value<0||value>999) value=0; else valid=true;
					sprintf(text,"%d",(int)value);
					wsprintf(BufferValue, TEXT("%S"),text);
				}
			}
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetTaskSpeedName()));
			break;

		// B17
		case LK_SPEEDTASK_AVG:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				// LKTOKEN  _@M1035_ = "Task Speed Average", _@M1036_ = "TskSpAv"
				_stprintf(BufferTitle, gettext(TEXT("_@M1036_")));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			value=0;
			if ( ActiveWayPoint >=1) {
				if ( ValidTaskPoint(ActiveWayPoint) ) {
					value = TASKSPEEDMODIFY*CALCULATED_INFO.TaskSpeed;
					if (value<=0||value>999) value=0; else valid=true;
					sprintf(text,"%d",(int)value);
					wsprintf(BufferValue, TEXT("%S"),text);
				}
			}
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetTaskSpeedName()));
			break;


		// B132 Final arrival with MC 0 , no totaly energy.
		case LK_FIN_ALTDIFF0:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				// LKTOKEN  _@M1191_ = "TskArr0"
				_stprintf(BufferTitle, gettext(TEXT("_@M1191_")));
			else
				// LKTOKEN  _@M1191_ = "TskArr0"
				_stprintf(BufferTitle, gettext(TEXT("_@M1191_")));
			if ( ValidTaskPoint(ActiveWayPoint) != false ) {
				index = Task[ActiveWayPoint].Index;
				if (index>=0) {
					value=ALTITUDEMODIFY*DerivedDrawInfo.TaskAltitudeDifference0;
					if ( value > ALTDIFFLIMIT ) {
						valid=true;
						_stprintf(BufferValue,TEXT("%+1.0f"), value);
					}
				}
			}
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			break;

		// B41	091006 using new task ete 
		case LK_FIN_ETE:
		// B133  091222 using old ETE corrected now
		case LK_LKFIN_ETE:
			wsprintf(BufferValue,_T(NULLTIME)); // 091222
			if (lktitle)
				// LKTOKEN  _@M1083_ = "Task Time To Go", _@M1084_ = "TskETE"
				_stprintf(BufferTitle, gettext(TEXT("_@M1084_")));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[LK_FIN_ETE].Title );

			if ( ValidTaskPoint(ActiveWayPoint) ) { // 091222
				if (CALCULATED_INFO.LKTaskETE > 0) { 
					valid=true;
					if ( Units::TimeToTextDown(BufferValue, (int)CALCULATED_INFO.LKTaskETE))  // 091112
						wsprintf(BufferUnit, TEXT("h"));
					else
						wsprintf(BufferUnit, TEXT(""));
				} else {
					index = Task[ActiveWayPoint].Index;
					if ( (WayPointCalc[index].NextETE > 0) && !ValidTaskPoint(1) ) {
						valid=true;
						if (Units::TimeToTextDown(BufferValue, (int)WayPointCalc[index].NextETE))
                                                	wsprintf(BufferUnit, TEXT("h"));
                                        	else
                                        	        wsprintf(BufferUnit, TEXT(""));
					} else
						wsprintf(BufferValue, TEXT(NULLTIME));
				}
			}
			// wsprintf(BufferUnit, TEXT("h")); 091112 REMOVE
			break;

		// B42
		case LK_NEXT_ETE:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				// LKTOKEN  _@M1085_ = "Next Time To Go", _@M1086_ = "NextETE"
				_stprintf(BufferTitle, gettext(TEXT("_@M1086_")));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );

			index = Task[ActiveWayPoint].Index;
			if ( (ValidTaskPoint(ActiveWayPoint) != false) && (WayPointCalc[index].NextETE < 0.9*ERROR_TIME)) {

				if (WayPointCalc[index].NextETE > 0) {
					valid=true;
					if (Units::TimeToTextDown(BufferValue, (int)WayPointCalc[index].NextETE)) // 091112
						wsprintf(BufferUnit, TEXT("h"));
					else
						wsprintf(BufferUnit, TEXT("m"));
				} else
					wsprintf(BufferValue, TEXT(NULLTIME));
			}
			// wsprintf(BufferUnit, TEXT("h")); 091112
			break;


		// B45
		case LK_FIN_ETA:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				// LKTOKEN  _@M1091_ = "Task Arrival Time", _@M1092_ = "TskETA"
				_stprintf(BufferTitle, gettext(TEXT("_@M1092_")));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );

			if ( (ValidTaskPoint(ActiveWayPoint) != false) && (CALCULATED_INFO.TaskTimeToGo< 0.9*ERROR_TIME)) {
				if (CALCULATED_INFO.TaskTimeToGo > 0) {
					valid=true;
					Units::TimeToText(BufferValue, (int)CALCULATED_INFO.TaskTimeToGo+DetectCurrentTime());
				} else
					wsprintf(BufferValue, TEXT(NULLTIME));
			}
			wsprintf(BufferUnit, TEXT("h"));
			break;


		// B46
		case LK_NEXT_ETA:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				// LKTOKEN  _@M1093_ = "Next Arrival Time", _@M1094_ = "NextETA"
				_stprintf(BufferTitle, gettext(TEXT("_@M1094_")));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );

			if ( (ValidTaskPoint(ActiveWayPoint) != false) && (CALCULATED_INFO.LegTimeToGo< 0.9*ERROR_TIME)) {
				if (CALCULATED_INFO.LegTimeToGo > 0) {
					valid=true;
					Units::TimeToText(BufferValue, (int)CALCULATED_INFO.LegTimeToGo+DetectCurrentTime());
				} else
					wsprintf(BufferValue, TEXT(NULLTIME));
			}
			wsprintf(BufferUnit, TEXT("h"));
			break;


		// B36		TmFly
		case LK_TIMEFLIGHT:
			wsprintf(BufferValue,_T(NULLTIME));
			if (lktitle)
				// LKTOKEN  _@M1073_ = "Time of flight", _@M1074_ = "FlyTime"
				_stprintf(BufferTitle, gettext(TEXT("_@M1074_")));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );

			if (CALCULATED_INFO.FlightTime > 0) {
				valid=true;
				Units::TimeToText(BufferValue, (int)CALCULATED_INFO.FlightTime);
			} else {
				wsprintf(BufferValue, TEXT(NULLTIME));
			}
			wsprintf(BufferUnit, TEXT("h"));
			break;

		// B09		Last thermal time
		case LK_TL_TIME:
			wsprintf(BufferValue,_T(NULLTIME));
			if (lktitle)
				// LKTOKEN  _@M1019_ = "Thermal Time Last", _@M1020_ = "TL.Time"
				_stprintf(BufferTitle, gettext(TEXT("_@M1020_")));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );

			if (CALCULATED_INFO.LastThermalTime > 0) {
				valid=true;
				Units::TimeToText(BufferValue, (int)CALCULATED_INFO.LastThermalTime);
			} else {
				wsprintf(BufferValue, TEXT(NULLTIME));
			}
			wsprintf(BufferUnit, TEXT("h"));
			break;


		// B27
		case LK_AA_TIME:
			wsprintf(BufferValue,_T(NULLTIME));
			_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );

#if (0)
			double dd;
			if (AATEnabled && ValidTaskPoint(ActiveWayPoint)) {
				dd = CALCULATED_INFO.TaskTimeToGo;
				if ((CALCULATED_INFO.TaskStartTime>0.0) && (CALCULATED_INFO.Flying) &&(ActiveWayPoint>0)) {
					dd += GPS_INFO.Time-CALCULATED_INFO.TaskStartTime;
				}
				dd= max(0,min(24.0*3600.0,dd))-AATTaskLength*60;
				if (dd<0) {
					status = 1; // red
				} else {
					if (CALCULATED_INFO.TaskTimeToGoTurningNow > (AATTaskLength+5)*60) {
						status = 2; // blue
					} else {
						status = 0;  // black
					}
				}
			} else {
				dd = 0;
				status = 0; // black
			}
#endif
			if (ValidTaskPoint(ActiveWayPoint) && AATEnabled && (CALCULATED_INFO.AATTimeToGo< 0.9*ERROR_TIME)) {

				Units::TimeToText(BufferValue, (int)CALCULATED_INFO.AATTimeToGo);
				valid=true;
			}
			wsprintf(BufferUnit,_T("h"));
			break;

		// B37
		case LK_GLOAD:
			//wsprintf(BufferValue,_T(NULLMEDIUM)); 100302 obs
			//wsprintf(BufferUnit,_T("g")); 100302 obs
			// _stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title ); 100302 obs
			if ( GPS_INFO.AccelerationAvailable) { 
				value=GPS_INFO.Gload;
				_stprintf(BufferValue,TEXT("%+.1f"), value);
				valid=true;
				// LKTOKEN  _@M1075_ = "G load", _@M1076_ = "G"
				_tcscpy(BufferTitle, gettext(TEXT("_@M1076_")));
			} else {
				value=CALCULATED_INFO.Gload;
				_stprintf(BufferValue,TEXT("%+.1f"), value);
				valid=true;
				_stprintf(BufferTitle, TEXT("e%s"), gettext(TEXT("_@M1076_")));
			}
			break;

		// B65 FIXED 100125
		case LK_BATTERY:
			wsprintf(BufferValue,_T(NULLMEDIUM));
			_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );

#if (WINDOWSPC<1)
	#ifndef GNAV
			value = PDABatteryPercent;
                	if (value<1||value>100)
				_stprintf(BufferValue,_T("---"));
                	else {
				if (PDABatteryFlag==BATTERY_FLAG_CHARGING || PDABatteryStatus==AC_LINE_ONLINE) {
					_stprintf(BufferValue,TEXT("%2.0f%%C"), value);	 // 100228
				} else {
					_stprintf(BufferValue,TEXT("%2.0f%%D"), value);  // 100228
				}

				valid = true;
			}
	#else
			value = GPS_INFO.SupplyBatteryVoltage;
			if (value>0.0) {
				_stprintf(BufferValue,TEXT("%2.1fV"), value);
				valid = true;
			} else {
				valid = false;
			}
	#endif
#endif
			break;


		// B62
		case LK_AA_DELTATIME:
			wsprintf(BufferValue,_T(NULLTIME));
			_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			wsprintf(BufferUnit,_T("h"));
			// TODO This is in the wrong place, should be moved to calc thread! 090916
			double dd;
			if (AATEnabled && ValidTaskPoint(ActiveWayPoint)) {
				dd = CALCULATED_INFO.TaskTimeToGo;
				if ((CALCULATED_INFO.TaskStartTime>0.0) && (CALCULATED_INFO.Flying) &&(ActiveWayPoint>0)) {
					dd += GPS_INFO.Time-CALCULATED_INFO.TaskStartTime;
				}
				dd= max(0,min(24.0*3600.0,dd))-AATTaskLength*60;
#if (0)
				if (dd<0) {
					status = 1; // red
				} else {
					if (CALCULATED_INFO.TaskTimeToGoTurningNow > (AATTaskLength+5)*60) {
						status = 2; // blue
					} else {
						status = 0;  // black
					}
				}
#endif
				if (dd < (0.9*ERROR_TIME)) {
					valid=true;
					Units::TimeToText(BufferValue, (int)dd);
				}
			}
			break;

#if 0
		// B133  091222 using old ETE corrected now
		case LK_LKFIN_ETE:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				// LKTOKEN  _@M1083_ = "Task Time To Go", _@M1084_ = "TskETE"
				_stprintf(BufferTitle, gettext(TEXT("_@M1084_")));
			else
				// LKTOKEN  _@M1083_ = "Task Time To Go", _@M1084_ = "TskETE"
				_stprintf(BufferTitle, gettext(TEXT("_@M1084_")));

			if ( (ValidTaskPoint(ActiveWayPoint) != false) && (CALCULATED_INFO.LKTaskETE< 0.9*ERROR_TIME)) {
				if (CALCULATED_INFO.LKTaskETE > 0) {
					Units::TimeToText(BufferValue, (int)CALCULATED_INFO.LKTaskETE);
					valid=true;
				} else
					wsprintf(BufferValue, TEXT(NULLTIME));
			}
			wsprintf(BufferUnit, TEXT("h"));
			break;
#endif


		// B134
		// Using MC=0!  total energy disabled
		case LK_NEXT_ALTDIFF0:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				// LKTOKEN  _@M1190_ = "ArrMc0"
				_stprintf(BufferTitle, gettext(TEXT("_@M1190_")));
			else
				// LKTOKEN  _@M1190_ = "ArrMc0"
				_stprintf(BufferTitle, gettext(TEXT("_@M1190_")));
			if ( ValidTaskPoint(ActiveWayPoint) != false ) {
				index = Task[ActiveWayPoint].Index;
				if (index>=0) {
					value=ALTITUDEMODIFY*DerivedDrawInfo.NextAltitudeDifference0;
					if ( value > ALTDIFFLIMIT ) {
						valid=true;
						_stprintf(BufferValue,TEXT("%+1.0f"), value);
					}
				}
			}
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			break;

		// B67
		case LK_ALTERN1_GR:
		// B68
		case LK_ALTERN2_GR:
		// B69
		case LK_BESTALTERN_GR:
			wsprintf(BufferValue,_T(NULLMEDIUM));
			if (lktitle) {
				switch (lkindex) {
					case LK_BESTALTERN_GR:
						// LKTOKEN  _@M1139_ = "BestAltern Req.Efficiency", _@M1140_ = "BAtn.E"
						_stprintf(BufferTitle, gettext(TEXT("_@M1140_")));
						break;
					case LK_ALTERN1_GR:
						// LKTOKEN  _@M1135_ = "Alternate1 Req.Efficiency", _@M1136_ = "Atn1.E"
						_stprintf(BufferTitle, gettext(TEXT("_@M1136_")));
						break;
					case LK_ALTERN2_GR:
						// LKTOKEN  _@M1137_ = "Alternate2 Req.Efficiency", _@M1138_ = "Atn2.E"
						_stprintf(BufferTitle, gettext(TEXT("_@M1138_")));
						break;

					default:
						_stprintf(BufferTitle, TEXT("Atn%d.E"), lkindex-LK_ALTERNATESGR+1);
						break;
				}//sw
			} else {
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			}
			switch(lkindex) {
				case LK_ALTERN1_GR:
					index=Alternate1;
					break;
				case LK_ALTERN2_GR:
					index=Alternate2;
					break;
				case LK_BESTALTERN_GR:
					index=BestAlternate;
					break;
				default:
					index=0;
					break;
			}

			if(ValidWayPoint(index))
			{
				if ( DisplayTextType == DISPLAYFIRSTTHREE)
				{
					 _tcsncpy(BufferTitle,WayPointList[index].Name,3);
					BufferTitle[3] = '\0';
				}
				else if( DisplayTextType == DISPLAYNUMBER) {
					_stprintf(BufferTitle,TEXT("%d"), WayPointList[index].Number );
				} else {
					_tcsncpy(BufferTitle,WayPointList[index].Name, 12);
					// BufferTitle[(sizeof(Text)/sizeof(TCHAR))-1] = '\0';
					if (lktitle)
						BufferTitle[12] = '\0'; // FIX TUNING
					else
						BufferTitle[8] = '\0';  // FIX TUNING
				}
			}
			// it would be time to use Alternate[] ..
			switch (lkindex) {
				case LK_ALTERN1_GR:
					if ( ValidWayPoint(Alternate1) ) value=WayPointCalc[Alternate1].GR;
					else value=INVALID_GR;
					break;
				case LK_ALTERN2_GR:
					if ( ValidWayPoint(Alternate2) ) value=WayPointCalc[Alternate2].GR;
					else value=INVALID_GR;
					break;
				case LK_BESTALTERN_GR:
					if ( ValidWayPoint(BestAlternate) ) value=WayPointCalc[BestAlternate].GR;
					else value=INVALID_GR;
					break;
				default:
					value = 1;
					break;
			}
			if (value <1 || value >=ALTERNATE_MAXVALIDGR ) {
				strcpy(text,NULLMEDIUM);
				valid=false;
			} else {
				if (value >= 100) sprintf(text,"%.0lf",value);
					else sprintf(text,"%.1lf",value);
				valid=true;
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("")); // 091227 BUGFIX
			break;

		// B75
		case LK_ALTERN1_ARRIV:
		// B76
		case LK_ALTERN2_ARRIV:
		// B77
		case LK_BESTALTERN_ARRIV:
			wsprintf(BufferValue,_T(NULLMEDIUM));
			if (lktitle) {
				switch (lkindex) {
					case LK_BESTALTERN_ARRIV:
						// LKTOKEN  _@M1155_ = "BestAlternate Arrival", _@M1156_ = "BAtnArr"
						_stprintf(BufferTitle, gettext(TEXT("_@M1156_")));
						break;
					case LK_ALTERN1_ARRIV:
						// LKTOKEN  _@M1151_ = "Alternate1 Arrival", _@M1152_ = "Atn1Arr"
						_stprintf(BufferTitle, gettext(TEXT("_@M1152_")));
						break;
					case LK_ALTERN2_ARRIV:
						// LKTOKEN  _@M1153_ = "Alternate2 Arrival", _@M1154_ = "Atn2Arr"
						_stprintf(BufferTitle, gettext(TEXT("_@M1154_")));
						break;
					default:
						_stprintf(BufferTitle, TEXT("Atn%dArr"), lkindex-LK_ALTERNATESARRIV+1);
						break;
				} //sw
			} else {
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			}
			switch(lkindex) {
				case LK_ALTERN1_ARRIV:
					index=Alternate1;
					break;
				case LK_ALTERN2_ARRIV:
					index=Alternate2;
					break;
				case LK_BESTALTERN_ARRIV:
					index=BestAlternate;
					break;
				default:
					index=0;
					break;
			}

			if(ValidWayPoint(index))
			{
				if ( DisplayTextType == DISPLAYFIRSTTHREE)
				{
					 _tcsncpy(BufferTitle,WayPointList[index].Name,3);
					BufferTitle[3] = '\0';
				}
				else if( DisplayTextType == DISPLAYNUMBER) {
					_stprintf(BufferTitle,TEXT("%d"), WayPointList[index].Number );
				} else {
					_tcsncpy(BufferTitle,WayPointList[index].Name, 12);
					// BufferTitle[(sizeof(Text)/sizeof(TCHAR))-1] = '\0';
					if (lktitle)
						BufferTitle[12] = '\0'; // FIX TUNING
					else
						BufferTitle[8] = '\0';  // FIX TUNING
				}
			}
			switch (lkindex) {
				case LK_ALTERN1_ARRIV:
					if ( ValidWayPoint(Alternate1) ) value=ALTITUDEMODIFY*WayPointCalc[Alternate1].AltArriv[AltArrivMode];
					else value=INVALID_DIFF;
					break;
				case LK_ALTERN2_ARRIV:
					if ( ValidWayPoint(Alternate2) ) value=ALTITUDEMODIFY*WayPointCalc[Alternate2].AltArriv[AltArrivMode];
					else value=INVALID_DIFF;
					break;
				case LK_BESTALTERN_ARRIV:
					if ( ValidWayPoint(BestAlternate) ) value=ALTITUDEMODIFY*WayPointCalc[BestAlternate].AltArriv[AltArrivMode];
					else value=INVALID_DIFF;
					break;
				default:
					value = INVALID_DIFF;
					break;
			}
			if (value <= ALTDIFFLIMIT ) {
				strcpy(text,NULLLONG);
				valid=false;
			} else { // 091221
				if ( (value>-1 && value<=0) || (value>=0 && value<1))
					sprintf(text,"0");
				else {
					sprintf(text,"%+.0f",value);
				}
				valid=true;
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			break;

		// B80
		case LK_EXTBATTBANK:
			wsprintf(BufferValue,_T(NULLMEDIUM));
			_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			ivalue=GPS_INFO.ExtBatt_Bank;
			if (ivalue>0) {
				_stprintf(BufferValue,TEXT("%d"), ivalue); // 091101
				valid = true;
			} else {
				valid = false;
			}
			break;

		// B81
		// B82
		case LK_EXTBATT1VOLT:
		case LK_EXTBATT2VOLT:
			wsprintf(BufferValue,_T(NULLMEDIUM));
			_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			value = (lkindex==LK_EXTBATT1VOLT?GPS_INFO.ExtBatt1_Voltage:GPS_INFO.ExtBatt2_Voltage);
			// hack to display percentage instead of voltage
			if (value>=1000) {
				value-=1000;
				if (value>0.0) {
					_stprintf(BufferValue,TEXT("%.0f%%"), value);
					valid = true;
				} else {
					valid = false;
				}
			} else {
				if (value>0.0) {
					_stprintf(BufferValue,TEXT("%0.2fv"), value);
					valid = true;
				} else {
					valid = false;
				}
			}
			break;

		// B48 091216  OAT Outside Air Temperature
		case LK_OAT:
			value=GPS_INFO.OutsideAirTemperature;
			_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			if (value<-50||value>100) {
				wsprintf(BufferValue, TEXT("---"));
			} else {
                	        sprintf(text,"%.0lf",value);
                	        wsprintf(BufferValue, TEXT("%S%S"),text,_T(DEG));
                	}
			break;

		// B84  100126
		case LK_AQNH:
			if (lktitle)
				// LKTOKEN  _@M1169_ = "Altern QNH", _@M1170_ = "aAlt"
				_stprintf(BufferTitle, gettext(TEXT("_@M1170_")));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			if (ALTITUDEMODIFY==TOMETER)
				value=TOFEET*DerivedDrawInfo.NavAltitude;
			else
				value=TOMETER*DerivedDrawInfo.NavAltitude;
			valid=true;
			sprintf(text,"%d",(int)value);
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetInvAltitudeName()));
			break;

		// B85  100126
		case LK_AALTAGL:
			if (lktitle)
				// LKTOKEN  _@M1171_ = "Altern AGL", _@M1172_ = "aHAGL"
				_stprintf(BufferTitle, gettext(TEXT("_@M1172_")));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			if (!CALCULATED_INFO.TerrainValid) { //@ 101013
				wsprintf(BufferValue, TEXT(NULLLONG));
				wsprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
				valid=false;
				break;
			}
			if (ALTITUDEMODIFY==TOMETER)
				value=TOFEET*DerivedDrawInfo.AltitudeAGL;
			else
				value=TOMETER*DerivedDrawInfo.AltitudeAGL;
			valid=true;
			sprintf(text,"%d",(int)value);
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetInvAltitudeName()));
			break;

		// B136
		case LK_TARGET_DIST:
			if (LKTargetIndex<0 || LKTargetIndex>=MAXTRAFFIC) {
					strcpy(text,NULLMEDIUM);
			} else {
	                        if (GPS_INFO.FLARM_Traffic[LKTargetIndex].ID <=0) {
					strcpy(text,NULLMEDIUM);
				} else {
					// get values
	                        	value=DISTANCEMODIFY*LKTraffic[LKTargetIndex].Distance;
					if (value>99) {
						strcpy(text,NULLMEDIUM);
					} else {
						valid=true;
						sprintf(text,"%.1f",value);
					}
				}
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));
			// LKTOKEN  _@M1023_ = "Next Distance", _@M1024_ = "Dist"
			wsprintf(BufferTitle, gettext(TEXT("_@M1024_")),text);
			break;

		// B137
		case LK_TARGET_TO:
			if (LKTargetIndex<0 || LKTargetIndex>=MAXTRAFFIC) {
					strcpy(text,NULLMEDIUM);
					wsprintf(BufferValue, TEXT("%S"),text);
			} else {
	                        if (GPS_INFO.FLARM_Traffic[LKTargetIndex].ID <=0) {
					strcpy(text,NULLMEDIUM);
					wsprintf(BufferValue, TEXT("%S"),text);
				} else {
						value = LKTraffic[LKTargetIndex].Bearing -  GPS_INFO.TrackBearing;
						valid=true;
						if (value < -180.0)
							value += 360.0;
						else
							if (value > 180.0)
								value -= 360.0;
						if (value > 1)
						_stprintf(BufferValue, TEXT("%2.0f°»"), value);
						else if (value < -1)
						_stprintf(BufferValue, TEXT("«%2.0f°"), -value);
						else
							_tcscpy(BufferValue, TEXT("«»"));
				}
			}
			wsprintf(BufferUnit, TEXT(""));
			// LKTOKEN  _@M1095_ = "Bearing Difference", _@M1096_ = "To"
			_stprintf(BufferTitle, gettext(TEXT("_@M1096_")));
			break;

		// B138
		case LK_TARGET_BEARING:
			if (LKTargetIndex<0 || LKTargetIndex>=MAXTRAFFIC) {
					strcpy(text,NULLMEDIUM);
					wsprintf(BufferValue, TEXT("%S"),text);
			} else {
	                        if (GPS_INFO.FLARM_Traffic[LKTargetIndex].ID <=0) {
					strcpy(text,NULLMEDIUM);
					wsprintf(BufferValue, TEXT("%S"),text);
				} else {
						value = LKTraffic[LKTargetIndex].Bearing;
						valid=true;
						if (value == 360)
							_stprintf(BufferValue, TEXT("0°"));
						else 
							_stprintf(BufferValue, TEXT("%2.0f°"), value);
				}
			}
			wsprintf(BufferUnit, TEXT(""));
			// LKTOKEN  _@M1007_ = "Bearing", _@M1008_ = "Brg"
			wsprintf(BufferTitle, gettext(TEXT("_@M1008_")));
			break;

		// B139
		case LK_TARGET_SPEED:
			if (LKTargetIndex<0 || LKTargetIndex>=MAXTRAFFIC) {
					strcpy(text,NULLMEDIUM);
			} else {
	                        if (GPS_INFO.FLARM_Traffic[LKTargetIndex].ID <=0) {
					strcpy(text,NULLMEDIUM);
				} else {
					value=SPEEDMODIFY*GPS_INFO.FLARM_Traffic[LKTargetIndex].Speed;
					if (value<0||value>9999) value=0; else valid=true;
					sprintf(text,"%d",(int)value);
				}
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetHorizontalSpeedName()));
			// LKTOKEN  _@M1013_ = "Speed ground", _@M1014_ = "GS"
			wsprintf(BufferTitle, gettext(TEXT("_@M1014_")));
			break;

		// B140
		case LK_TARGET_ALT:
			if (LKTargetIndex<0 || LKTargetIndex>=MAXTRAFFIC) {
					strcpy(text,NULLMEDIUM);
			} else {
	                        if (GPS_INFO.FLARM_Traffic[LKTargetIndex].ID <=0) {
					strcpy(text,NULLMEDIUM);
				} else {
					value=ALTITUDEMODIFY*GPS_INFO.FLARM_Traffic[LKTargetIndex].Altitude;
					valid=true;
					sprintf(text,"%d",(int)value);
				}
			}
			// LKTOKEN  _@M1001_ = "Altitude QNH", _@M1002_ = "Alt", 
			_stprintf(BufferTitle, gettext(TEXT("_@M1002_")));
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			break;

		// B141
		// DO NOT USE RELATIVE ALTITUDE: when not real time, it won't change in respect to our position!!!
		// This is negative when target is below us because it represent a remote position
		case LK_TARGET_ALTDIFF:
			if (LKTargetIndex<0 || LKTargetIndex>=MAXTRAFFIC) {
					strcpy(text,NULLMEDIUM);
			} else {
	                        if (GPS_INFO.FLARM_Traffic[LKTargetIndex].ID <=0) {
					strcpy(text,NULLMEDIUM);
				} else {
					value=ALTITUDEMODIFY*(DerivedDrawInfo.NavAltitude-GPS_INFO.FLARM_Traffic[LKTargetIndex].Altitude)*-1;
					valid=true;
					sprintf(text,"%+d",(int)value);
				}
			}
			// LKTOKEN  _@M1193_ = "RelAlt"
			_stprintf(BufferTitle, gettext(TEXT("_@M1193_")));
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			break;

		// B142
		case LK_TARGET_VARIO:
			if (LKTargetIndex<0 || LKTargetIndex>=MAXTRAFFIC) {
					strcpy(text,NULLMEDIUM);
					wsprintf(BufferValue, TEXT("%S"),text);
					_tcscpy(BufferUnit, _T(""));
			} else {
	                        if (GPS_INFO.FLARM_Traffic[LKTargetIndex].ID <=0) {
					strcpy(text,NULLMEDIUM);
					wsprintf(BufferValue, TEXT("%S"),text);
					_tcscpy(BufferUnit, _T(""));
				} else {
					value = LIFTMODIFY*GPS_INFO.FLARM_Traffic[LKTargetIndex].ClimbRate;
					valid=true;
					_stprintf(BufferValue,varformat,value);
					wsprintf(BufferUnit, TEXT("%s"),Units::GetVerticalSpeedName());
				}
			}
			// LKTOKEN  _@M1194_ = "Var"
			_tcscpy(BufferTitle, gettext(TEXT("_@M1194_")));
			break;

		// B143
		case LK_TARGET_AVGVARIO:
			if (LKTargetIndex<0 || LKTargetIndex>=MAXTRAFFIC) {
					strcpy(text,NULLMEDIUM);
					wsprintf(BufferValue, TEXT("%S"),text);
					_tcscpy(BufferUnit, _T(""));
			} else {
	                        if (GPS_INFO.FLARM_Traffic[LKTargetIndex].ID <=0) {
					strcpy(text,NULLMEDIUM);
					wsprintf(BufferValue, TEXT("%S"),text);
					_tcscpy(BufferUnit, _T(""));
				} else {
					value = LIFTMODIFY*GPS_INFO.FLARM_Traffic[LKTargetIndex].Average30s;
					valid=true;
					_stprintf(BufferValue,varformat,value);
					wsprintf(BufferUnit, TEXT("%s"),Units::GetVerticalSpeedName());
				}
			}
			// LKTOKEN  _@M1189_ = "Var30"
			_tcscpy(BufferTitle, gettext(TEXT("_@M1189_")));
			break;

		// B144
		case LK_TARGET_ALTARRIV:
			wsprintf(BufferValue,_T(NULLLONG));
			// LKTOKEN  _@M1188_ = "Arr"
			_stprintf(BufferTitle, gettext(TEXT("_@M1188_")));
			if (LKTargetIndex<0 || LKTargetIndex>=MAXTRAFFIC) {
					strcpy(text,NULLMEDIUM);
					wsprintf(BufferValue, TEXT("%S"),text);
			} else {
	                        if (GPS_INFO.FLARM_Traffic[LKTargetIndex].ID <=0) {
					strcpy(text,NULLMEDIUM);
					wsprintf(BufferValue, TEXT("%S"),text);
				} else {

					value=ALTITUDEMODIFY*LKTraffic[LKTargetIndex].AltArriv;
					if ( value > ALTDIFFLIMIT ) {
						valid=true;
						_stprintf(BufferValue,TEXT("%+1.0f"), value);
					} else {
						strcpy(text,NULLMEDIUM);
						wsprintf(BufferValue, TEXT("%S"),text);
					}
				}
			}
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			break;

		// B145
		case LK_TARGET_GR:
			wsprintf(BufferValue,_T(NULLLONG));
			// LKTOKEN  _@M1187_ = "ReqE"
			_stprintf(BufferTitle, gettext(TEXT("_@M1187_")));
			_tcscpy(BufferUnit,_T(""));
			if (LKTargetIndex<0 || LKTargetIndex>=MAXTRAFFIC) {
					strcpy(text,NULLMEDIUM);
			} else {
	                        if (GPS_INFO.FLARM_Traffic[LKTargetIndex].ID <=0) {
					strcpy(text,NULLMEDIUM);
					wsprintf(BufferValue, TEXT("%S"),text);
				} else {
					value=LKTraffic[LKTargetIndex].GR;
					if (value <1 || value >=ALTERNATE_MAXVALIDGR )
						strcpy(text,NULLMEDIUM);
					else {
						if (value >= 100) sprintf(text,"%.0lf",value);
							else sprintf(text,"%.1lf",value);
						valid=true;
					}
				}
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			break;

		// B146
		case LK_TARGET_EIAS:
			// LKTOKEN  _@M1186_ = "eIAS"
			_stprintf(BufferTitle, gettext(TEXT("_@M1186_")));
			_tcscpy(BufferUnit,_T(""));
			if (LKTargetIndex<0 || LKTargetIndex>=MAXTRAFFIC) {
					strcpy(text,NULLMEDIUM);
			} else {
				value=SPEEDMODIFY*LKTraffic[LKTargetIndex].EIAS;
				if (value<0||value>999) value=0; else valid=true;
				sprintf(text,"%d",(int)value);
				wsprintf(BufferUnit, TEXT("%s"),(Units::GetHorizontalSpeedName()));
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			break;

		case LK_DUMMY:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				_stprintf(BufferTitle, TEXT("Dummy"));
			else
				_stprintf(BufferTitle, TEXT("Dummy"));

			wsprintf(BufferUnit, TEXT("."));
			break;

		case LK_EMPTY:
			wsprintf(BufferValue, TEXT(""));
			//wsprintf(BufferUnit, TEXT(""));
			wsprintf(BufferTitle, TEXT(""));
			break;
		case LK_ERROR:
			// let it be shown entirely to understand the problem
			valid=true;
			wsprintf(BufferValue, TEXT("000"));
			wsprintf(BufferUnit, TEXT("e"));
			wsprintf(BufferTitle, TEXT("Err"));
			break;
		default:
			valid=false;
			wsprintf(BufferValue, TEXT(NULLMEDIUM));
			wsprintf(BufferUnit, TEXT("."));
			if ( lkindex >=NUMSELECTSTRINGS || lkindex <1 ) 
				wsprintf(BufferTitle, TEXT("BadErr"));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;
	}

  return valid;
}

// simple format distance value for a given index. BufferTitle always NULLed
// wpindex is a WayPointList index
// wpvirtual true means virtual waypoint, and relative special checks

void MapWindow::LKFormatDist(const int wpindex, const bool wpvirtual, TCHAR *BufferValue, TCHAR *BufferUnit) {

  static int	index;
  static double value;
  static char	text[LKSIZEBUFFERVALUE];

  index=-1;

  if (wpvirtual) {
	if ( ValidResWayPoint(wpindex) ) index = wpindex;
  } else {
	if ( ValidWayPoint(wpindex) ) index = wpindex;
  }
  if (index>=0) {
	value=WayPointCalc[index].Distance*DISTANCEMODIFY;
	if (value>99)
		sprintf(text,"%.0f",value);
	else
		sprintf(text,"%.1f",value);
  } else {
	strcpy(text,NULLMEDIUM);
  }
  wsprintf(BufferValue, TEXT("%S"),text);
  wsprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));
  return;
}

void MapWindow::LKFormatBrgDiff(const int wpindex, const bool wpvirtual, TCHAR *BufferValue, TCHAR *BufferUnit) {

  static int	index;
  static double value;

  index=-1;

  if (wpvirtual) {
	if ( ValidResWayPoint(wpindex) ) index = wpindex;
  } else {
	if ( ValidWayPoint(wpindex) ) index = wpindex;
  }
  _tcscpy(BufferValue,_T(NULLMEDIUM)); 
  _tcscpy(BufferUnit,_T(""));
  if (index>=0) {
#ifndef MAP_ZOOM
	if (DisplayMode != dmCircling) {
#else /* MAP_ZOOM */
	if (!MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING)) {
#endif /* MAP_ZOOM */
		value = WayPointCalc[index].Bearing -  GPS_INFO.TrackBearing;
		if (value < -180.0)
			value += 360.0;
		else
			if (value > 180.0)
				value -= 360.0;
#ifndef __MINGW32__
		if (value > 1)
			_stprintf(BufferValue, TEXT("%2.0f\xB0\xBB"), value);
		else if (value < -1)
			_stprintf(BufferValue, TEXT("\xAB%2.0f\xB0"), -value);
		else
			_tcscpy(BufferValue, TEXT("\xAB\xBB"));
#else
		if (value > 1)
			_stprintf(BufferValue, TEXT("%2.0f°»"), value);
		else if (value < -1)
			_stprintf(BufferValue, TEXT("«%2.0f°"), -value);
		else
			_tcscpy(BufferValue, TEXT("«»"));
	}
#endif
  }
}


void MapWindow::LKFormatGR(const int wpindex, const bool wpvirtual, TCHAR *BufferValue, TCHAR *BufferUnit) {

  static int	index;
  static double value;
  static char	text[LKSIZEBUFFERVALUE];

  index=-1;

  if (wpvirtual) {
	if ( ValidResWayPoint(wpindex) ) index = wpindex;
  } else {
	if ( ValidWayPoint(wpindex) ) index = wpindex;
  }
  _tcscpy(BufferValue,_T(NULLMEDIUM)); 
  _tcscpy(BufferUnit,_T(""));

  if (index>=0) {
	value=WayPointCalc[index].GR;
  } else {
	value=INVALID_GR;
  }

  if (value >=1 && value <MAXEFFICIENCYSHOW ) {
	if (value >= 100)
		sprintf(text,"%.0lf",value);
	else
		sprintf(text,"%.1lf",value);

	wsprintf(BufferValue, TEXT("%S"),text);
  }
}

void MapWindow::LKFormatAltDiff(const int wpindex, const bool wpvirtual, TCHAR *BufferValue, TCHAR *BufferUnit) {

  static int	index;
  static double value;
  static char	text[LKSIZEBUFFERVALUE];

  index=-1;

  if (wpvirtual) {
	if ( ValidResWayPoint(wpindex) ) index = wpindex;
  } else {
	if ( ValidWayPoint(wpindex) ) index = wpindex;
  }
  _tcscpy(BufferValue,_T(NULLMEDIUM)); 
  wsprintf(BufferUnit, _T("%s"),(Units::GetAltitudeName()));

  if (index>=0) {
	value=WayPointCalc[index].AltArriv[AltArrivMode]*ALTITUDEMODIFY;
  } else {
	value=INVALID_DIFF;
  }

  if (value > ALTDIFFLIMIT ) {
	if ( value>-1 && value<1 )
		sprintf(text,"0");
	else
		sprintf(text,"%+.0f",value);

	wsprintf(BufferValue, TEXT("%S"),text);
  }
}
