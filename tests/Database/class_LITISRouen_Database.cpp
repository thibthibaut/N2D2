/*
    (C) Copyright 2016 CEA LIST. All Rights Reserved.
    Contributor(s): Olivier BICHLER (olivier.bichler@cea.fr)

    This software is governed by the CeCILL-C license under French law and
    abiding by the rules of distribution of free software.  You can  use,
    modify and/ or redistribute the software under the terms of the CeCILL-C
    license as circulated by CEA, CNRS and INRIA at the following URL
    "http://www.cecill.info".

    As a counterpart to the access to the source code and  rights to copy,
    modify and redistribute granted by the license, users are provided only
    with a limited warranty  and the software's author,  the holder of the
    economic rights,  and the successive licensors  have only  limited
    liability.

    The fact that you are presently reading this means that you have had
    knowledge of the CeCILL-C license and that you accept its terms.
*/

#include "N2D2.hpp"

#include "Database/LITISRouen_Database.hpp"
#include "utils/UnitTest.hpp"

using namespace N2D2;

TEST(LITISRouen_Database, load)
{
    REQUIRED(UnitTest::DirExists(N2D2_DATA("data_rouen")));

    LITISRouen_Database db(0.4, 0.4);
    db.load(N2D2_DATA("data_rouen"));

    const unsigned int nbStimuli = 3026;
    const unsigned int nbLabels = 19;

    ASSERT_EQUALS(db.getNbStimuli(), nbStimuli);
    ASSERT_EQUALS(db.getNbStimuli(Database::Learn),
                  (unsigned int)(0.4 * nbStimuli));
    ASSERT_EQUALS(db.getNbStimuli(Database::Validation),
                  (unsigned int)(0.4 * nbStimuli) + 1);
    ASSERT_EQUALS(db.getNbStimuli(Database::Test),
                  (unsigned int)(0.2 * nbStimuli));
    ASSERT_EQUALS(db.getNbStimuli(Database::Unpartitioned), 0U);
    ASSERT_EQUALS(db.getNbStimuli(Database::Learn)
                  + db.getNbStimuli(Database::Validation)
                  + db.getNbStimuli(Database::Test),
                  nbStimuli);
    ASSERT_EQUALS(db.getNbLabels(), nbLabels);

    ASSERT_EQUALS(db.getLabelName(0), "avion");
    ASSERT_EQUALS(db.getNbStimuliWithLabel(0), 23U);
    ASSERT_EQUALS(db.getLabelName(18), "tubestation");
    ASSERT_EQUALS(db.getNbStimuliWithLabel(18), 125U);
}

RUN_TESTS()
