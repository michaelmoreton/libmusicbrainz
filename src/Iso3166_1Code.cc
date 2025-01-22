/* --------------------------------------------------------------------------

   libmusicbrainz5 - Client library to access MusicBrainz

   Copyright (C) 2012 Andrew Hawkins, Mike Moreton

   This file is part of libmusicbrainz5.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   libmusicbrainz5 is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this library.  If not, see <http://www.gnu.org/licenses/>.

     $Id$

----------------------------------------------------------------------------*/

#include "config.h"
#include "musicbrainz5/defines.h"

#include "musicbrainz5/Iso3166_1Code.h"

#include <string.h>

class MusicBrainz5::CIso3166_1CodePrivate
{
	public:
		CIso3166_1CodePrivate()
		{
		}

		std::string m_Code;
};

MusicBrainz5::CIso3166_1Code::CIso3166_1Code(const XMLNode& Node)
:	CEntity(),
	m_d(new CIso3166_1CodePrivate)
{
	if (!Node.isEmpty())
	{
		//std::cout << "Iso3166_1Code node: " << std::endl << Node.createXMLString(true) << std::endl;

		Parse(Node);

		if (Node.getText()) {
			ProcessItem(Node,m_d->m_Code);
		}
	}
}

MusicBrainz5::CIso3166_1Code::CIso3166_1Code(const CIso3166_1Code& Other)
:	CEntity(),
	m_d(new CIso3166_1CodePrivate)
{
	*this=Other;
}

MusicBrainz5::CIso3166_1Code& MusicBrainz5::CIso3166_1Code::operator =(const CIso3166_1Code& Other)
{
	if (this!=&Other)
	{
		Cleanup();

		CEntity::operator =(Other);

		m_d->m_Code=Other.m_d->m_Code;
	}

	return *this;
}

MusicBrainz5::CIso3166_1Code::~CIso3166_1Code()
{
	Cleanup();

	delete m_d;
}

void MusicBrainz5::CIso3166_1Code::Cleanup()
{
}

MusicBrainz5::CIso3166_1Code *MusicBrainz5::CIso3166_1Code::Clone()
{
	return new CIso3166_1Code(*this);
}

void MusicBrainz5::CIso3166_1Code::ParseAttribute(const std::string& Name, const std::string& Value)
{
#ifdef _MB5_DEBUG_
	std::cerr << "Unrecognised release attribute: '" << Name << "'" << std::endl;
#else
	(void)Name;
	(void)Value;
#endif
}

void MusicBrainz5::CIso3166_1Code::ParseElement(const XMLNode& Node)
{
	std::string NodeName=Node.getName();

#ifdef _MB5_DEBUG_
	std::cerr << "Unrecognised release event element: '" << NodeName << "'" << std::endl;
#endif
}

std::string MusicBrainz5::CIso3166_1Code::GetElementName()
{
	return "iso-3166-1-code";
}

std::string MusicBrainz5::CIso3166_1Code::Code() const
{
	return m_d->m_Code;
}

std::ostream& MusicBrainz5::CIso3166_1Code::Serialise(std::ostream& os) const
{
	os << "Release event:" << std::endl;

	CEntity::Serialise(os);

	os << "\tCode:                " << Code() << std::endl;

	return os;
}
