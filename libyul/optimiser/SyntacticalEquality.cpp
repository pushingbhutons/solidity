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
	return boost::apply_visitor([this](auto&& _lhs, auto&& _rhs) -> bool {
		// ``this->`` is redundant, but required to work around a bug present in gcc 6.x.
		return this->expressionEqual(_lhs, _rhs);
	}, _lhs, _rhs);
}

bool SyntacticallyEqual::operator()(Statement const& _lhs, Statement const& _rhs)
{
	return boost::apply_visitor([this](auto&& _lhs, auto&& _rhs) -> bool {
		// ``this->`` is redundant, but required to work around a bug present in gcc 6.x.
		return this->statementEqual(_lhs, _rhs);
	}, _lhs, _rhs);
}

bool SyntacticallyEqual::expressionEqual(FunctionalInstruction const& _lhs, FunctionalInstruction const& _rhs)
{
	return
		_lhs.instruction == _rhs.instruction &&
		compareVector<Expression, &SyntacticallyEqual::operator()>(_lhs.arguments, _rhs.arguments)
	;
}

bool SyntacticallyEqual::expressionEqual(FunctionCall const& _lhs, FunctionCall const& _rhs)
{
	return
		expressionEqual(_lhs.functionName, _rhs.functionName) &&
		compareVector<Expression, &SyntacticallyEqual::operator()>(_lhs.arguments, _rhs.arguments)
	;
}

bool SyntacticallyEqual::expressionEqual(Identifier const& _lhs, Identifier const& _rhs)
{
	auto lhsIt = m_identifiersLHS.find(_lhs.name);
	auto rhsIt = m_identifiersRHS.find(_rhs.name);
	return
		(lhsIt == m_identifiersLHS.end() && rhsIt == m_identifiersRHS.end() && _lhs.name == _rhs.name) ||
		(lhsIt != m_identifiersLHS.end() && rhsIt != m_identifiersRHS.end() && lhsIt->second == rhsIt->second)
	;
}
bool SyntacticallyEqual::expressionEqual(Literal const& _lhs, Literal const& _rhs)
{
	return _lhs.kind == _rhs.kind && _lhs.value == _rhs.value && _lhs.type == _rhs.type;
}

bool SyntacticallyEqual::statementEqual(ExpressionStatement const& _lhs, ExpressionStatement const& _rhs)
{
	return (*this)(_lhs.expression, _rhs.expression);
}
bool SyntacticallyEqual::statementEqual(Assignment const& _lhs, Assignment const& _rhs)
{
	return
		compareVector<Identifier, &SyntacticallyEqual::expressionEqual>(_lhs.variableNames, _rhs.variableNames) &&
		compareSharedPtr<Expression, &SyntacticallyEqual::operator()>(_lhs.value, _rhs.value)
	;
}

bool SyntacticallyEqual::statementEqual(VariableDeclaration const& _lhs, VariableDeclaration const& _rhs)
{
	// first visit expression, then variable declarations
	if (!compareSharedPtr<Expression, &SyntacticallyEqual::operator()>(_lhs.value, _rhs.value))
		return false;
	return compareVector<TypedName, &SyntacticallyEqual::visitDeclaration>(_lhs.variables, _rhs.variables);
}

bool SyntacticallyEqual::statementEqual(FunctionDefinition const& _lhs, FunctionDefinition const& _rhs)
{
	// first visit parameter declarations, then body
	if (!compareVector<TypedName, &SyntacticallyEqual::visitDeclaration>(_lhs.parameters, _rhs.parameters))
		return false;
	if (!compareVector<TypedName, &SyntacticallyEqual::visitDeclaration>(_lhs.returnVariables, _rhs.returnVariables))
		return false;
	return statementEqual(_lhs.body, _rhs.body);
}

bool SyntacticallyEqual::statementEqual(If const& _lhs, If const& _rhs)
{
	return
		compareSharedPtr<Expression, &SyntacticallyEqual::operator()>(_lhs.condition, _rhs.condition) &&
		statementEqual(_lhs.body, _rhs.body)
	;
}

bool SyntacticallyEqual::statementEqual(Switch const& _lhs, Switch const& _rhs)
{
	return
		compareSharedPtr<Expression, &SyntacticallyEqual::operator()>(_lhs.expression, _rhs.expression) &&
		compareVector<Case, &SyntacticallyEqual::switchCaseEqual>(_lhs.cases, _rhs.cases)
	;
}


bool SyntacticallyEqual::switchCaseEqual(Case const& _lhs, Case const& _rhs)
{
	return
		compareSharedPtr<Literal, &SyntacticallyEqual::expressionEqual>(_lhs.value, _rhs.value) &&
		statementEqual(_lhs.body, _rhs.body)
	;
}

bool SyntacticallyEqual::statementEqual(ForLoop const& _lhs, ForLoop const& _rhs)
{
	return
		statementEqual(_lhs.pre, _rhs.pre) &&
		compareSharedPtr<Expression, &SyntacticallyEqual::operator()>(_lhs.condition, _rhs.condition) &&
		statementEqual(_lhs.body, _rhs.body) &&
		statementEqual(_lhs.post, _rhs.post)
	;
}

bool SyntacticallyEqual::statementEqual(Instruction const&, Instruction const&)
{
	assertThrow(false, OptimizerException, "");
}

bool SyntacticallyEqual::statementEqual(Label const&, Label const&)
{
	assertThrow(false, OptimizerException, "");
}

bool SyntacticallyEqual::statementEqual(StackAssignment const&, StackAssignment const&)
{
	assertThrow(false, OptimizerException, "");
}

bool SyntacticallyEqual::statementEqual(Block const& _lhs, Block const& _rhs)
{
	return compareVector<Statement, &SyntacticallyEqual::operator()>(_lhs.statements, _rhs.statements);
}

bool SyntacticallyEqual::visitDeclaration(TypedName const& _lhs, TypedName const& _rhs)
{
	if (_lhs.type != _rhs.type)
		return false;
	std::size_t id = m_idsUsed++;
	m_identifiersLHS[_lhs.name] = id;
	m_identifiersRHS[_rhs.name] = id;
	return true;
}
