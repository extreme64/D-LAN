/**
  * D-LAN - A decentralized LAN file sharing software.
  * Copyright (C) 2010-2012 Greg Burri <greg.burri@gmail.com>
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program.  If not, see <http://www.gnu.org/licenses/>.
  */
  
#include <QCoreApplication>
#include <QTest>
#include <QString>
#include <QStringList>

#include <Tests.h>
#include <TreeTests.h>
#include <BenchmarkTests.h>

int main(int argc, char* argv[])
{
   QStringList args;
   for (int i = 1; i < argc; i++)
      args << argv[i];

   if (args.contains("benchmark"))
   {
      BenchmarkTests benchmarkTests;
      return QTest::qExec(&benchmarkTests, argc, argv);
   }
   else
   {
      Tests tests;
      TreeTests treeTests;
      return QTest::qExec(&tests, argc, argv) + QTest::qExec(&treeTests, argc, argv);
   }
}
