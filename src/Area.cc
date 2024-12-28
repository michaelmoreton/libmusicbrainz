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

#include "musicbrainz5/Area.h"

#include <string.h>

#include "musicbrainz5/Iso3166_1CodeList.h"

class MusicBrainz5::CAreaPrivate
{
	public:
		CAreaPrivate()
		:	m_Iso3166_1CodeList(0)
		{
		}

		std::string m_ID;
		std::string m_Name;
		std::string m_SortName;
		CIso3166_1CodeList *m_Iso3166_1CodeList;
};

MusicBrainz5::CArea::CArea(const XMLNode& Node)
:	CEntity(),
	m_d(new CAreaPrivate)
{
	if (!Node.isEmpty())
	{
		//std::cout << "Area node: " << std::endl << Node.createXMLString(true) << std::endl;

		Parse(Node);
	}
}

MusicBrainz5::CArea::CArea(const CArea& Other)
:	CEntity(),
	m_d(new CAreaPrivate)
{
	*this=Other;
}

MusicBrainz5::CArea& MusicBrainz5::CArea::operator =(const CArea& Other)
{
	if (this!=&Other)
	{
		Cleanup();

		CEntity::operator =(Other);

		m_d->m_ID=Other.m_d->m_ID;
		m_d->m_Name=Other.m_d->m_Name;
		m_d->m_SortName=Other.m_d->m_SortName;

		if (Other.m_d->m_Iso3166_1CodeList)
			m_d->m_Iso3166_1CodeList=new CIso3166_1CodeList(*Other.m_d->m_Iso3166_1CodeList);

	}

	return *this;
}

MusicBrainz5::CArea::~CArea()
{
	Cleanup();

	delete m_d;
}

void MusicBrainz5::CArea::Cleanup()
{
	delete m_d->m_Iso3166_1CodeList;
	m_d->m_Iso3166_1CodeList=0;
}

MusicBrainz5::CArea *MusicBrainz5::CArea::Clone()
{
	return new CArea(*this);
}

void MusicBrainz5::CArea::ParseAttribute(const std::string& Name, const std::string& Value)
{
	if ("id"==Name)
		m_d->m_ID=Value;
	else
	{
#ifdef _MB5_DEBUG_
		std::cerr << "Unrecognised release attribute: '" << Name << "'" << std::endl;
#endif
	}
}

void MusicBrainz5::CArea::ParseElement(const XMLNode& Node)
{
	std::string NodeName=Node.getName();

	if ("name"==NodeName)
	{
		ProcessItem(Node,m_d->m_Name);
	}
	else if ("sort-name"==NodeName)
	{
		ProcessItem(Node,m_d->m_SortName);
	}
	else if ("iso-3166-1-code-list"==NodeName)
	{
		ProcessItem(Node,m_d->m_Iso3166_1CodeList);
	}
	else
	{
#ifdef _MB5_DEBUG_
		std::cerr << "Unrecognised release element: '" << NodeName << "'" << std::endl;
#endif
	}
}

std::string MusicBrainz5::CArea::GetElementName()
{
	return "Area";
}

std::string MusicBrainz5::CArea::ID() const
{
	return m_d->m_ID;
}

std::string MusicBrainz5::CArea::Name() const
{
	return m_d->m_Name;
}

std::string MusicBrainz5::CArea::SortName() const
{
	return m_d->m_SortName;
}

MusicBrainz5::CIso3166_1CodeList *MusicBrainz5::CArea::Iso3166_1CodeList() const
{
	return m_d->m_Iso3166_1CodeList;
}

std::ostream& MusicBrainz5::CArea::Serialise(std::ostream& os) const
{
	os << "Area:" << std::endl;

	CEntity::Serialise(os);

	os << "\tID:                  " << ID() << std::endl;
	os << "\tName:                " << Name() << std::endl;
	os << "\tSortName:            " << SortName() << std::endl;

	if (Iso3166_1CodeList())
		os << *Iso3166_1CodeList() << std::endl;

	return os;
}
