/*
 * =====================================================================================
 *
 *       Filename:  variablesmap.h
 *
 * 		 Description:  libglademm variablesmap 的复制品
 *
 *        Version:  1.0
 *        Created:  2009年08月10日 19时45分58秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  wind (xihe), xihels@gmail.com
 *        Company:  cyclone
 *
 * =====================================================================================
 */
#ifndef VARIABLEMAP_HH_
#define VARIABLEMAP_HH_

#include <gtkmm.h>
#include <map>
class VariablesMap {
	public:
		explicit VariablesMap(const Glib::RefPtr<Gtk::Builder>& glade);
		virtual ~VariablesMap();

		///For ToggleButton (CheckBox and RadioButton)
		virtual void connect_widget(const Glib::ustring& widget_name, bool& variable);

		///For Entry, ComboBoxEntry and SpinBox
		virtual void connect_widget(const Glib::ustring& widget_name, Glib::ustring& variable);

		///For Scale (HScale and VScale)
		virtual void connect_widget(const Glib::ustring& widget_name, double& variable);

		///For Calendar
		virtual void connect_widget(const Glib::ustring& widget_name, Glib::Date& variable);

		///Transfer data from the widget to the variable.
		virtual void transfer_widgets_to_variables();

		///Transfer data from the variable to the widget.
		virtual void transfer_variables_to_widgets();

	protected:

		/** Override this to validate the data that the user enters into the widgets.
		 * The return value indicates whether the widgets' data is valid.
		 */
		virtual bool validate_widgets();

		virtual void transfer_one_widget(Gtk::Widget* pWidget, bool to_variable);

		typedef std::map<Gtk::Widget*, void*> type_mapWidgetsToVariables;
		type_mapWidgetsToVariables m_mapWidgetsToVariables;

		Glib::RefPtr<Gtk::Builder> m_refGlade;
};

#endif
