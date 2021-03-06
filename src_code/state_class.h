#ifndef _STATE_CLASS_H_
#define _STATE_CLASS_H_

/****************************************************
    State Class

    File:   state_class.h
    Author: James Stokebrand
    jamesstokebrand AT gmail DOT com

    state_class.h file is part of the RGB LED Controller and Node 
     version 1 hardware project.

    This file is a helper class that will work behind the scenes to help 
      implement state machines.

    Copyright (C) 2015 - James Stokebrand - 2015 Mar 12

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    Rev History:
     When         Who                Description of change
    -----------  ----------         ------------------------
    2014 Oct 05  James Stokebrand   Initial creation.

*****************************************************/

#include <avr/io.h>

#ifndef _EVENT_LISTING_H_
#include "event_listing.h"
#endif

class base_state_class
{
public:
    //  STATE method pointer typedef
    typedef void (base_state_class::*STATE)(const event_element_class &A);

    base_state_class(STATE init) 
    : state(init) 
    , transition(false)
    {
        ENTER_EVENT.set(E_STATE_MACHINE,E_ENTER_STATE);
        EXIT_EVENT.set(E_STATE_MACHINE,E_EXIT_STATE);

        // Execute the ENTER transition for the inital state
        //(this->*state)(ENTER_EVENT); // REMOVED!

        // ENTER_EVENT initalization should be triggered in the drived object and not here!
    }
    virtual ~base_state_class() {}

    // Process is called with new events pushed into
    //  the state machine.
    void process(const event_element_class &A);

    // This method is to be called internal to the state machine to 
    //   perform a state transition once we exit the current
    //   state.
    void TRAN(volatile STATE target) { 
        next_state = target;
        transition = true;
    }

    // This method is to be called external to the state machine to 
    //  force an immediate state transition.
    void EXTERNAL_TRAN(volatile STATE target);


    volatile STATE state;
    volatile STATE next_state;

    volatile bool transition;

    event_element_class ENTER_EVENT;
    event_element_class EXIT_EVENT;
};

#endif

