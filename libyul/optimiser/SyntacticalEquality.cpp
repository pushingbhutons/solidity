/*(
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
 * Component that can compare ASTs for equality on a syntactic basis.
 */

#include <libyul/optimiser/SyntacticalEquality.h>

#include <libyul/Exceptions.h>
#include <libyul/AsmData.h>

#include <libdevcore/CommonData.h>

using namespace std;
using namespace dev;
using namespace yul;

bool SyntacticallyEqual::operator()(Expression const& _lhs, Expression const& _rhs)
{
	return boost::apply_visitor([this](auto&& _lhs, auto&& _rhs) -> bool { return (*this)(_lhs, _rhs); }, _lhs, _rhs);
}

bool SyntacticallyEqual::operator()(Statement const& _lhs, Statement const& _rhs)
{
	return boost::apply_visitor([this](auto&& _lhs, auto&& _rhs) -> bool { return (*this)(_lhs, _rhs); }, _lhs, _rhs);
}

bool SyntacticallyEqual::operator()(FunctionalInstruction const& _lhs, FunctionalInstruction const& _rhs)
{
	return _lhs.instruction == _rhs.instruction && (*this)(_lhs.arguments, _rhs.arguments);
}

bool SyntacticallyEqual::operator()(FunctionCall const& _lhs, FunctionCall const& _rhs)
{
	return (*this)(_lhs.functionName, _rhs.functionName) && (*this)(_lhs.arguments, _rhs.arguments);
}

bool SyntacticallyEqual::operator()(Identifier const& _lhs, Identifier const& _rhs)
{
	auto lhsIt = m_identifiersLHS.find(_lhs.name);
	auto rhsIt = m_identifiersRHS.find(_rhs.name);
	return
		(lhsIt == m_identifiersLHS.end() && rhsIt == m_identifiersRHS.end() && _lhs.name == _rhs.name) ||
		(lhsIt != m_identifiersLHS.end() && rhsIt != m_identifiersRHS.end() && lhsIt->second == rhsIt->second)
	;
}

bool SyntacticallyEqual::operator()(Literal const& _lhs, Literal const& _rhs)
{
	return _lhs.kind == _rhs.kind && _lhs.value == _rhs.value && _lhs.type == _rhs.type;
}

bool SyntacticallyEqual::operator()(ExpressionStatement const& _lhs, ExpressionStatement const& _rhs)
{
	return (*this)(_lhs.expression, _rhs.expression);
}
bool SyntacticallyEqual::operator()(Assignment const& _lhs, Assignment const& _rhs)
{
	return (*this)(_lhs.variableNames, _rhs.variableNames) && (*this)(*_lhs.value, *_rhs.value);
}

bool SyntacticallyEqual::operator()(VariableDeclaration const& _lhs, VariableDeclaration const& _rhs)
{
	return (*this)(_lhs.variables, _rhs.variables) && (*this)(_lhs.value, _rhs.value);
}

bool SyntacticallyEqual::operator()(FunctionDefinition const& _lhs, FunctionDefinition const& _rhs)
{
	// Make sure to consume the declarations of arguments and return variables before consuming the function body.
	if (!(*this)(_lhs.parameters, _rhs.parameters))
		return false;
	if (!(*this)(_lhs.returnVariables, _rhs.returnVariables))
		return false;
	return (*this)(_lhs.body, _rhs.body);
}

bool SyntacticallyEqual::operator()(If const& _lhs, If const& _rhs)
{
	return (*this)(_lhs.condition, _rhs.condition) && (*this)(_lhs.body, _rhs.body);
}

bool SyntacticallyEqual::operator()(Switch const& _lhs, Switch const& _rhs)
{
	return (*this)(_lhs.expression, _rhs.expression) && (*this)(_lhs.cases, _rhs.cases);
}

bool SyntacticallyEqual::operator()(ForLoop const& _lhs, ForLoop const& _rhs)
{
	return
		(*this)(_lhs.pre, _rhs.pre) &&
		(*this)(_lhs.condition, _rhs.condition) &&
		(*this)(_lhs.body, _rhs.body) &&
		(*this)(_lhs.post, _rhs.post)
	;
}

bool SyntacticallyEqual::operator()(Block const& _lhs, Block const& _rhs)
{
	return (*this)(_lhs.statements, _rhs.statements);
}

bool SyntacticallyEqual::operator()(TypedName const& _lhs, TypedName const& _rhs)
{
	if (_lhs.type != _rhs.type)
		return false;
	std::size_t id = m_idsUsed++;
	m_identifiersLHS[_lhs.name] = id;
	m_identifiersRHS[_rhs.name] = id;
	return true;
}

bool SyntacticallyEqual::operator()(Case const& _lhs, Case const& _rhs)
{
	return (*this)(_lhs.value, _rhs.value) && (*this)(_lhs.body, _rhs.body);
}

bool SyntacticallyEqual::operator()(Instruction const&, Instruction const&)
{
	assertThrow(false, OptimizerException, "");
}

bool SyntacticallyEqual::operator()(Label const&, Label const&)
{
	assertThrow(false, OptimizerException, "");
}

bool SyntacticallyEqual::operator()(StackAssignment const&, StackAssignment const&)
{
	assertThrow(false, OptimizerException, "");
}
