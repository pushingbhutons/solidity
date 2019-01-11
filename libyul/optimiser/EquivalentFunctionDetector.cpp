/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * Optimiser component that combines syntactically equivalent functions.
 */

#include <libyul/optimiser/EquivalentFunctionDetector.h>
#include <libyul/optimiser/SyntacticalEquality.h>

#include <libyul/AsmData.h>
#include <libyul/optimiser/Metrics.h>

using namespace std;
using namespace dev;
using namespace yul;
using namespace solidity;

void EquivalentFunctionDetector::operator()(FunctionDefinition const& _fun)
{
	RoughHeuristic heuristic(_fun);
	auto it = m_candidates.emplace(heuristic, std::initializer_list<FunctionDefinition const*>{&_fun});
	if (!it.second)
	{
		for (auto const& candidate: it.first->second)
			if (SyntacticallyEqual{}.statementEqual(_fun, *candidate))
			{
				m_duplicates[_fun.name] = candidate;
				return;
			}
		it.first->second.push_back(&_fun);
	}
}

bool EquivalentFunctionDetector::RoughHeuristic::operator<(EquivalentFunctionDetector::RoughHeuristic const& _rhs) const
{
	if (m_fun.parameters.size() < _rhs.m_fun.parameters.size())
		return true;
	if (_rhs.m_fun.parameters.size() < m_fun.parameters.size())
		return false;
	if (m_fun.returnVariables.size() < _rhs.m_fun.returnVariables.size())
		return true;
	if (_rhs.m_fun.returnVariables.size() < m_fun.returnVariables.size())
		return false;
	if (codeSize() < _rhs.codeSize())
		return true;
	return _rhs.codeSize() < codeSize();
}

size_t EquivalentFunctionDetector::RoughHeuristic::codeSize() const
{
	if (!m_codeSize)
		m_codeSize = CodeSize::codeSize(m_fun.body);
	return *m_codeSize;
}
