/*
 * =====================================================================================
 *
 *       Filename:  variablesmap.cpp
 *
 *    Description:  ligblademm variablesmap 的复制品
 *
 *        Version:  1.0
 *        Created:  2009年08月10日 19时44分57秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  wind (xihe), xihels@gmail.com
 *        Company:  cyclone
 *
 * =====================================================================================
 */

#include "variablesmap.h"

VariablesMap::VariablesMap(const Glib::RefPtr<Gtk::Builder>& glade)
: m_refGlade(glade)
{
}

VariablesMap::~VariablesMap()
{
}


void VariablesMap::connect_widget(const Glib::ustring& widget_name, bool& variable)
{
  Gtk::ToggleButton* pToggleButton = 0;
  m_refGlade->get_widget(widget_name, pToggleButton); //Gtk::Builder will complain if it is not a ToggleButton.
  if(pToggleButton)
  {
    m_mapWidgetsToVariables[pToggleButton] = (void*)(&variable);
  }
}

void VariablesMap::connect_widget(const Glib::ustring& widget_name, Glib::ustring& variable)
{
  Gtk::Widget* pWidget = 0;
  m_refGlade->get_widget(widget_name, pWidget); 

  Gtk::Entry* pEntry = dynamic_cast<Gtk::Entry*>(pWidget); //it mange both Gtk::entry and Gtk::SpinButton
  Gtk::ComboBoxEntry* pComboBoxEntry = dynamic_cast<Gtk::ComboBoxEntry*>(pWidget);
  if(pEntry)
  {
    m_mapWidgetsToVariables[pEntry] = (void*)(&variable);
  }
  if(pComboBoxEntry)
  {
    m_mapWidgetsToVariables[pComboBoxEntry] = (void*)(&variable);
  }
}

void VariablesMap::connect_widget(const Glib::ustring& widget_name, double& variable)
{
  Gtk::Widget* pWidget = 0;
  m_refGlade->get_widget(widget_name, pWidget); 

  Gtk::Scale* pScale = dynamic_cast<Gtk::Scale*>(pWidget); 
  if(pScale)
  {
    m_mapWidgetsToVariables[pScale] = (void*)(&variable);
  }
}

void VariablesMap::connect_widget(const Glib::ustring& widget_name, Glib::Date& variable)
{
  Gtk::Widget* pWidget = 0;
  m_refGlade->get_widget(widget_name, pWidget); 

  Gtk::Calendar* pCalendar = dynamic_cast<Gtk::Calendar*>(pWidget); 
  if(pCalendar)
  {
    m_mapWidgetsToVariables[pCalendar] = (void*)(&variable);
  }
}

void VariablesMap::transfer_widgets_to_variables()
{
  if(validate_widgets()) //If the widgets' data is correct. Useful to override.
  {
    for(type_mapWidgetsToVariables::iterator iter =  m_mapWidgetsToVariables.begin(); iter != m_mapWidgetsToVariables.end(); ++iter)
    {
      transfer_one_widget(iter->first, true); //true = to_variable.
    }
  }
}

void VariablesMap::transfer_variables_to_widgets()
{
  for(type_mapWidgetsToVariables::iterator iter =  m_mapWidgetsToVariables.begin(); iter != m_mapWidgetsToVariables.end(); ++iter)
  {
    transfer_one_widget(iter->first, false); //false = to_widget.
  }
}


void VariablesMap::transfer_one_widget(Gtk::Widget* pWidget, bool to_variable)
{
  //Find the widget in the map:
  type_mapWidgetsToVariables::iterator iterFind = m_mapWidgetsToVariables.find(pWidget);
  if(iterFind != m_mapWidgetsToVariables.end())
  {
    //Get the variable for the widget:
    void* pVariable = iterFind->second;
    if(pVariable)
    {
      //Cast the variable appropriately and set it appropriately:
      Gtk::Entry* pEntry = dynamic_cast<Gtk::Entry*>(pWidget);
      Gtk::ComboBoxEntry* pComboBoxEntry = dynamic_cast<Gtk::ComboBoxEntry*>(pWidget);

      Gtk::ToggleButton* pToggleButton = dynamic_cast<Gtk::ToggleButton*>(pWidget); //CheckButtons and RadioButtons.
      Gtk::Scale* pScale = dynamic_cast<Gtk::Scale*>(pWidget); 
      Gtk::Calendar* pCalendar = dynamic_cast<Gtk::Calendar*>(pWidget); 

      if(pEntry)
      {
        Glib::ustring* pVar = (Glib::ustring*)(pVariable);

        if(to_variable)
          (*pVar) = pEntry->get_text();
        else
          pEntry->set_text(*pVar);
      }
      if(pComboBoxEntry)
      {
        Glib::ustring* pVar = (Glib::ustring*)(pVariable);
	Gtk::Entry* pIEntry = dynamic_cast<Gtk::Entry*>(pComboBoxEntry->get_child());

        if(to_variable){
	  if(pIEntry) 
          	(*pVar) = pIEntry->get_text();
        } else {
	  if(pIEntry) 
	          pIEntry->set_text(*pVar);
	}
      }
      if(pToggleButton)
      {
        bool* pVar = (bool*)(pVariable);

        if(to_variable)
          (*pVar) = pToggleButton->get_active();
        else
          pToggleButton->set_active(*pVar);
      }
      if(pScale)
      {
        double* pVar = (double*)(pVariable);

        if(to_variable)
          (*pVar) = pScale->get_value();
        else
          pScale->set_value(*pVar);
      }
      if(pCalendar)
      {
        Glib::Date* pVar = (Glib::Date*)(pVariable);

        if(to_variable){
	   guint year,month,day;
	   pCalendar->get_date(year,month,day);
          (*pVar) = Glib::Date(day,(Glib::Date::Month)month,year);
	} else {
          pCalendar->select_day(pVar->get_day());
          pCalendar->select_month(pVar->get_month(), pVar->get_year());
	}
      }
    }
  }
}

bool VariablesMap::validate_widgets()
{
  //Override to add validation.
  //TODO: We could add some automatic data-range and text-length validation.
  return true;
}



