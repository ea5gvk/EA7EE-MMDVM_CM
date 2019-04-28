/*
 *   Copyright (C) 2010-2014,2016,2017 by Jonathan Naylor G4KLX
 *   Copyright (C) 2019 by Manuel Sanchez EA7EE
 *   Copyright (C) 2018 by Andy Uribe CA6JAU
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "APRSWriter.h"

#include "YSFDefines.h"

#include <cstdio>
#include <cassert>
#include <cstring>
#include <cmath>

CAPRSWriter::CAPRSWriter(const std::string& callsign, const std::string& suffix, const std::string& password, const std::string& address, unsigned int port) :
m_thread(NULL),
m_enabled(false),
m_idTimer(1000U, 20U * 60U),		// 20 minutes
m_callsign(callsign),
m_txFrequency(0U),
m_rxFrequency(0U),
m_latitude(0.0F),
m_longitude(0.0F),
fm_latitude(0.0F),
fm_longitude(0.0F),
m_follow(false),
m_height(0),
m_desc()
{
	assert(!callsign.empty());
	assert(!password.empty());
	assert(!address.empty());
	assert(port > 0U);

	if (!suffix.empty()) {
		m_callsign.append("-");
		m_callsign.append(suffix.substr(0U, 1U));
	}

	m_thread = new CAPRSWriterThread(m_callsign, password, address, port);
}

CAPRSWriter::~CAPRSWriter()
{
}

void CAPRSWriter::setInfo(const std::string& node_callsign, unsigned int txFrequency, unsigned int rxFrequency, float latitude, float longitude, int height, const std::string& desc, const std::string& icon, const std::string& beacon_text, int beacon_time, bool follow)
{
	m_txFrequency = txFrequency;
	m_rxFrequency = rxFrequency;
	m_latitude    = latitude;
	m_longitude   = longitude;
	m_height      = height;
	m_desc        = desc;
	m_follow	  = follow;
	m_idTimer.start(60U * beacon_time);
	m_node_callsign = node_callsign;

	if (icon.length()>0) m_icon = icon;
	else m_icon = "YY";

	if (beacon_text.length()>0) m_beacon_text = beacon_text;
	else m_beacon_text = "YSF2DMR - Private HotSpot";

	sendIdFrames();
}

bool CAPRSWriter::open()
{
	m_idTimer.start();
	return m_thread->start();
}

void CAPRSWriter::write(const unsigned char* source, const char* type, unsigned char radio, float fLatitude, float fLongitude, unsigned int tg_qrv)
{
	assert(source != NULL);
	assert(type != NULL);

	char callsign[11U];
	
	strcpy(callsign, (const char *)source);
	unsigned int i=0;
	while ((callsign[i]!=' ') && (i<strlen(callsign))) i++;
	callsign[i]=0;

	if (strcmp(callsign,m_node_callsign.c_str())==0) {
		LogMessage("Catching %s position.",m_node_callsign.c_str());
		fm_latitude = fLatitude;
		fm_longitude = fLongitude;
	}

	::memcpy(callsign, source, YSF_CALLSIGN_LENGTH);
	callsign[YSF_CALLSIGN_LENGTH] = 0x00U;

	int n = ::strspn(callsign, "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
	callsign[n] = 0x00U;

	double tempLat = ::fabs(fLatitude);
	double tempLong = ::fabs(fLongitude);

	double latitude = ::floor(tempLat);
	double longitude = ::floor(tempLong);

	latitude = (tempLat - latitude)  * 60.0 + latitude  * 100.0;
	longitude = (tempLong - longitude) * 60.0 + longitude * 100.0;

	char lat[20U];
	::sprintf(lat, "%07.2lf", latitude);

	char lon[20U];
	::sprintf(lon, "%08.2lf", longitude);

	char symbol;
	char suffix[3];
	switch (radio) {
	case 0x24U:
	case 0x28U:
		symbol = '[';
		strcpy(suffix, "-7");
		break;
	case 0x25U:
	case 0x29U:
		symbol = '>';
		strcpy(suffix, "-9");
		break;
	case 0x26U:
		symbol = 'r';
		strcpy(suffix, "-1");
		break;
	case 0x27U:
		symbol = '-';
		strcpy(suffix, "-2");
		break;		
	default:
		symbol = '-';
		strcpy(suffix, "-2");
		break;
	}
	
	char output[300U];
	::sprintf(output, "%s%s>APDPRS,C4FM*,qAR,%s:!%s%c/%s%c%c %s QRV TG %d via MMDVM",
		callsign, suffix, m_callsign.c_str(),
		lat, (fLatitude < 0.0F) ? 'S' : 'N',
		lon, (fLongitude < 0.0F) ? 'W' : 'E',
		symbol, type, tg_qrv);

	m_thread->write(output);
}

void CAPRSWriter::clock(unsigned int ms)
{
	m_idTimer.clock(ms);

	if (m_idTimer.hasExpired()) {
		sendIdFrames();
		m_idTimer.start();
	}
}

void CAPRSWriter::close()
{
	m_thread->stop();
}

void CAPRSWriter::sendIdFrames()
{	
	// Default values aren't passed on
	if (m_latitude == 0.0F && m_longitude == 0.0F)
		return;

	double tempLat, tempLong;

	if (fm_latitude == 0.0F) tempLat  = ::fabs(m_latitude);
	else {
		//LogMessage("lat: %f",fm_latitude);
		tempLat  = ::fabs(fm_latitude);
	}

	if (fm_longitude == 0.0F) tempLong = ::fabs(m_longitude);
	else {
		//LogMessage("lon: %f",fm_longitude);
		tempLong = ::fabs(fm_longitude);
	}

	double latitude  = ::floor(tempLat);
	double longitude = ::floor(tempLong);

	latitude  = (tempLat  - latitude)  * 60.0 + latitude  * 100.0;
	longitude = (tempLong - longitude) * 60.0 + longitude * 100.0;

	char lat[20U];
	::sprintf(lat, "%07.2lf", latitude);

	char lon[20U];
	::sprintf(lon, "%08.2lf", longitude);

	char output[500U];
	char mobile[10];
	if (fm_latitude == 0.0F) strcpy(mobile,"");
	else strcpy(mobile," /mobile");
	::sprintf(output, "%s>APDG03,TCPIP*,qAC,%s:!%s%c%c%s%c%c%s%s",
	    m_node_callsign.c_str(), m_node_callsign.c_str(),
		lat, (m_latitude < 0.0F)  ? 'S' : 'N',(m_icon.c_str())[0],
		lon, (m_longitude < 0.0F) ? 'W' : 'E',(m_icon.c_str())[1],
		m_beacon_text.c_str(), mobile);

	m_thread->write(output);

	m_idTimer.start();
}
