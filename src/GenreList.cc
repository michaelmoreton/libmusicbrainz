/* --------------------------------------------------------------------------

   libmusicbrainz5 - Client library to access MusicBrainz

   Copyright (C) 2024 Andrew Hawkins, Mike Moreton

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

#include "musicbrainz5/GenreList.h"

#include "musicbrainz5/Genre.h"

class MusicBrainz5::CGenreListPrivate
{
	public:
		CGenreListPrivate()
		{
		}

};

MusicBrainz5::CGenreList::CGenreList(const XMLNode& Node)
:	CListImpl<CGenre>(),
	m_d(new CGenreListPrivate)
{
	if (!Node.isEmpty())
	{
		//std::cout << "GenreList node: " << std::endl << Node.createXMLString(true) << std::endl;

		Parse(Node);
	}
}

MusicBrainz5::CGenreList::CGenreList(const CGenreList& Other)
:	CListImpl<CGenre>(),
	m_d(new CGenreListPrivate)
{
	*this=Other;
}

MusicBrainz5::CGenreList& MusicBrainz5::CGenreList::operator =(const CGenreList& Other)
{
	if (this!=&Other)
	{
		CListImpl<CGenre>::operator =(Other);
	}

	return *this;
}

MusicBrainz5::CGenreList::~CGenreList()
{
	delete m_d;
}

MusicBrainz5::CGenreList *MusicBrainz5::CGenreList::Clone()
{
	return new CGenreList(*this);
}

void MusicBrainz5::CGenreList::ParseAttribute(const std::string& Name, const std::string& Value)
{
	CListImpl<CGenre>::ParseAttribute(Name,Value);
}

void MusicBrainz5::CGenreList::ParseElement(const XMLNode& Node)
{
	std::string NodeName=Node.getName();

	CListImpl<CGenre>::ParseElement(Node);
}

std::string MusicBrainz5::CGenreList::GetElementName()
{
	return "genre-list";
}

std::ostream& MusicBrainz5::CGenreList::Serialise(std::ostream& os) const
{
	os << "Genre list:" << std::endl;

	CListImpl<CGenre>::Serialise(os);

	return os;
}
