/*
 * Copyright (c) 2009, 2024, Oracle and/or its affiliates.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2.0, as
 * published by the Free Software Foundation.
 *
 * This program is designed to work with certain software (including
 * but not limited to OpenSSL) that is licensed under separate terms, as
 * designated in a particular file or component or in included license
 * documentation. The authors of MySQL hereby grant you an additional
 * permission to link the program and your derivative works with the
 * separately licensed software that they have either included with
 * the program or referenced in the documentation.
 *
 * Without limiting anything contained in the foregoing, this file,
 * which is part of Connector/C++, is also subject to the
 * Universal FOSS Exception, version 1.0, a copy of which can be found at
 * https://oss.oracle.com/licenses/universal-foss-exception.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License, version 2.0, for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */



#include "cppconn/exception.h"

#include "../BaseTestFixture.h"

/**
 *
 * @author Mark Matthews
 * @version $Id: TransactionTest.java,v 1.1.2.1 2005/05/13 18:58:37 mmatthews
 *          Exp $
 */

namespace testsuite
{
namespace simple
{
  class TransactionTest : public BaseTestFixture
  {
  private:
    typedef BaseTestFixture super;
  // ---------------------------------------------

    static const double DOUBLE_CONST;

/* throws sql::DbcException */

    void createTestTable() ;

protected:

  public:
    TEST_FIXTURE( TransactionTest )
    {
      TEST_CASE( testTransaction );
    }


    /* throws std::exception * */
    void setUp();

  /**
   * DOCUMENT ME!
   *
   * @throws sql::SQLException
   *             DOCUMENT ME!
   */
  /* throws sql::SQLException */

    void testTransaction();
  };

REGISTER_FIXTURE(TransactionTest);

}
}
