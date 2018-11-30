/*
Minetest
Copyright (C) 2013 celeron55, Perttu Ahola <celeron55@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#ifndef PROFILER_HPP_
#define PROFILER_HPP_

#include "irrlicht/irrlichttypes.h"
#include <cassert>
#include <string>
#include <map>
#include <ostream>

#include "threading/mutex_auto_lock.h"
#include "util/timetaker.h"
#include "util/numeric.h"      // paging()

#define MAX_PROFILER_TEXT_ROWS 20

// Global profiler
class Profiler;
extern Profiler *g_profiler;

/*
	Time profiler
*/

class Profiler {
	public:
		Profiler() = default;

		void add(const std::string &name, float value);
		void avg(const std::string &name, float value);

		void clear();

		void print(std::ostream &o);

		float getValue(const std::string &name) const;

		void printPage(std::ostream &o, u32 page, u32 pagecount);

		using GraphValues = std::map<std::string, float>;

		void graphAdd(const std::string &id, float value);
		void graphGet(GraphValues &result);

		void remove(const std::string& name);

	private:
		std::mutex m_mutex;
		std::map<std::string, float> m_data;
		std::map<std::string, int> m_avgcounts;
		std::map<std::string, float> m_graphvalues;
};

enum ScopeProfilerType {
	SPT_ADD,
	SPT_AVG,
	SPT_GRAPH_ADD
};

class ScopeProfiler {
	public:
		ScopeProfiler(Profiler *profiler, const std::string &name,
				ScopeProfilerType type = SPT_ADD);
		~ScopeProfiler();

	private:
		Profiler *m_profiler = nullptr;
		std::string m_name;
		TimeTaker *m_timer = nullptr;
		enum ScopeProfilerType m_type;
};

#endif // PROFILER_HPP_
