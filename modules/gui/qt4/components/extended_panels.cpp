/*****************************************************************************
 * extended_panels.cpp : Extended controls panels
 ****************************************************************************
 * Copyright (C) 2006-2007 the VideoLAN team
 * $Id$
 *
 * Authors: Clément Stenac <zorglub@videolan.org>
 *          Antoine Cellerier <dionoea .t videolan d@t org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * ( at your option ) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <QLabel>
#include <QVariant>
#include <QString>
#include <QFont>
#include <QGridLayout>
#include <QSignalMapper>
#include <QComboBox>

#include "components/extended_panels.hpp"
#include "dialogs/preferences.hpp"
#include "dialogs_provider.hpp"
#include "qt4.hpp"
#include "input_manager.hpp"

#include <vlc_aout.h>
#include <vlc_intf_strings.h>
#include <vlc_vout.h>
#include <vlc_osd.h>


#if 0
class ConfClickHandler : public QObject
{
public:
    ConfClickHandler( intf_thread_t *_p_intf, ExtVideo *_e ) : QObject ( _e ) {
        e = _e; p_intf = _p_intf;
    }
    virtual ~ConfClickHandler() {}
    bool eventFilter( QObject *obj, QEvent *evt )
    {
        if( evt->type() == QEvent::MouseButtonPress )
        {
            e->gotoConf( obj );
            return true;
        }
        return false;
    }
private:
    ExtVideo* e;
    intf_thread_t *p_intf;
};
#endif

QString ModuleFromWidgetName( QObject *obj )
{
    return obj->objectName().replace( "Enable","" );
}

QString OptionFromWidgetName( QObject *obj )
{
    /* Gruik ? ... nah */
    QString option = obj->objectName().replace( "Slider", "" )
                                      .replace( "Combo" , "" )
                                      .replace( "Dial"  , "" )
                                      .replace( "Check" , "" )
                                      .replace( "Spin"  , "" )
                                      .replace( "Text"  , "" );
    for( char a = 'A'; a <= 'Z'; a++ )
    {
        option = option.replace( QString( a ),
                                 QString( '-' ) + QString( a + 'a' - 'A' ) );
    }
    return option;
}

ExtVideo::ExtVideo( intf_thread_t *_p_intf, QTabWidget *_parent ) :
                           p_intf( _p_intf )
{
    ui.setupUi( _parent );

#define SETUP_VFILTER( widget ) \
    { \
        vlc_object_t *p_obj = ( vlc_object_t * ) \
            vlc_object_find_name( p_intf->p_libvlc, \
                                  #widget, \
                                  FIND_CHILD ); \
        QCheckBox *checkbox = qobject_cast<QCheckBox*>( ui.widget##Enable ); \
        QGroupBox *groupbox = qobject_cast<QGroupBox*>( ui.widget##Enable ); \
        if( p_obj ) \
        { \
            vlc_object_release( p_obj ); \
            if( checkbox ) checkbox->setChecked( true ); \
            else groupbox->setChecked( true ); \
        } \
        else \
        { \
            if( checkbox ) checkbox->setChecked( false ); \
            else groupbox->setChecked( false ); \
        } \
    } \
    CONNECT( ui.widget##Enable, clicked(), this, updateFilters() );
#define SETUP_VFILTER_OPTION( widget, signal ) \
    initComboBoxItems( ui.widget ); \
    setWidgetValue( ui.widget ); \
    CONNECT( ui.widget, signal, this, updateFilterOptions() );

    SETUP_VFILTER( adjust )
    SETUP_VFILTER_OPTION( hueSlider, valueChanged( int ) )
    SETUP_VFILTER_OPTION( contrastSlider, valueChanged( int ) )
    SETUP_VFILTER_OPTION( brightnessSlider, valueChanged( int ) )
    SETUP_VFILTER_OPTION( saturationSlider, valueChanged( int ) )
    SETUP_VFILTER_OPTION( gammaSlider, valueChanged( int ) )
    SETUP_VFILTER_OPTION( brightnessThresholdCheck, stateChanged( int ) )

    SETUP_VFILTER( extract )
    SETUP_VFILTER_OPTION( extractComponentText, textChanged( QString ) )

    SETUP_VFILTER( colorthres )
    SETUP_VFILTER_OPTION( colorthresColorText, textChanged( QString ) )
    SETUP_VFILTER_OPTION( colorthresSaturationthresSlider, valueChanged( int ) )
    SETUP_VFILTER_OPTION( colorthresSimilaritythresSlider, valueChanged( int ) )

    SETUP_VFILTER( invert )

    SETUP_VFILTER( gradient )
    SETUP_VFILTER_OPTION( gradientModeCombo, currentIndexChanged( QString ) )
    SETUP_VFILTER_OPTION( gradientTypeCheck, stateChanged( int ) )
    SETUP_VFILTER_OPTION( gradientCartoonCheck, stateChanged( int ) )

    SETUP_VFILTER( motionblur )
    SETUP_VFILTER_OPTION( blurFactorSlider, valueChanged( int ) )

    SETUP_VFILTER( motiondetect )

    SETUP_VFILTER( noise )

    SETUP_VFILTER( psychedelic )

    SETUP_VFILTER( sharpen )
    SETUP_VFILTER_OPTION( sharpenSigmaSlider, valueChanged( int ) )

    SETUP_VFILTER( ripple )

    SETUP_VFILTER( wave )

    SETUP_VFILTER( transform )
    SETUP_VFILTER_OPTION( transformTypeCombo, currentIndexChanged( QString ) )

    SETUP_VFILTER( rotate )
    SETUP_VFILTER_OPTION( rotateAngleDial, valueChanged( int ) )
    ui.rotateAngleDial->setWrapping( true );
    ui.rotateAngleDial->setNotchesVisible( true );

    SETUP_VFILTER( puzzle )
    SETUP_VFILTER_OPTION( puzzleRowsSpin, valueChanged( int ) )
    SETUP_VFILTER_OPTION( puzzleColsSpin, valueChanged( int ) )
    SETUP_VFILTER_OPTION( puzzleBlackSlotCheck, stateChanged( int ) )

    SETUP_VFILTER( magnify )

    SETUP_VFILTER( clone )
    SETUP_VFILTER_OPTION( cloneCountSpin, valueChanged( int ) )

    SETUP_VFILTER( wall )
    SETUP_VFILTER_OPTION( wallRowsSpin, valueChanged( int ) )
    SETUP_VFILTER_OPTION( wallColsSpin, valueChanged( int ) )

    SETUP_VFILTER( panoramix )
    SETUP_VFILTER_OPTION( panoramixRowsSpin, valueChanged( int ) )
    SETUP_VFILTER_OPTION( panoramixColsSpin, valueChanged( int ) )


    SETUP_VFILTER( erase )
    SETUP_VFILTER_OPTION( eraseMaskText, editingFinished() )
    SETUP_VFILTER_OPTION( eraseYSpin, valueChanged( int ) )
    SETUP_VFILTER_OPTION( eraseXSpin, valueChanged( int ) )

    SETUP_VFILTER( marq )
    SETUP_VFILTER_OPTION( marqMarqueeText, textChanged( QString ) )
    SETUP_VFILTER_OPTION( marqPositionCombo, currentIndexChanged( QString ) )

    SETUP_VFILTER( logo )
    SETUP_VFILTER_OPTION( logoFileText, editingFinished() )
    SETUP_VFILTER_OPTION( logoYSpin, valueChanged( int ) )
    SETUP_VFILTER_OPTION( logoXSpin, valueChanged( int ) )
    SETUP_VFILTER_OPTION( logoTransparencySlider, valueChanged( int ) )

#undef SETUP_VFILTER
#undef SETUP_VFILTER_OPTION

    CONNECT( ui.cropTopPx, valueChanged( int ), this, cropChange() );
    CONNECT( ui.cropBotPx, valueChanged( int ), this, cropChange() );
    CONNECT( ui.cropLeftPx, valueChanged( int ), this, cropChange() );
    CONNECT( ui.cropRightPx, valueChanged( int ), this, cropChange() );
    CONNECT( ui.topBotCropSync, toggled( bool ),
             ui.cropBotPx, setDisabled( bool ) );
    CONNECT( ui.leftRightCropSync, toggled( bool ),
             ui.cropRightPx, setDisabled( bool ) );
}

ExtVideo::~ExtVideo()
{
}

void ExtVideo::cropChange()
{
    p_vout = ( vout_thread_t * )vlc_object_find( p_intf,
                                VLC_OBJECT_VOUT, FIND_CHILD );
    if( p_vout )
    {
        var_SetInteger( p_vout, "crop-top", ui.cropTopPx->value() );
        var_SetInteger( p_vout, "crop-bottom", ui.cropBotPx->value() );
        var_SetInteger( p_vout, "crop-left", ui.cropLeftPx->value() );
        var_SetInteger( p_vout, "crop-right", ui.cropRightPx->value() );
    }
}

void ExtVideo::ChangeVFiltersString( char *psz_name, bool b_add )
{
    char *psz_parser, *psz_string;
    const char *psz_filter_type;

    /* Please leave p_libvlc_global. This is where cached modules are
     * stored. We're not trying to find a module instance. */
    module_t *p_obj = module_Find( p_intf, psz_name );
    if( !p_obj )
    {
        msg_Err( p_intf, "Unable to find filter module \"%s\n.", psz_name );
        return;
    }

    if( module_IsCapable( p_obj, "video filter2" ) )
    {
        psz_filter_type = "video-filter";
    }
    else if( module_IsCapable( p_obj, "video filter" ) )
    {
        psz_filter_type = "vout-filter";
    }
    else if( module_IsCapable( p_obj, "sub filter" ) )
    {
        psz_filter_type = "sub-filter";
    }
    else
    {
        module_Put( p_obj );
        msg_Err( p_intf, "Unknown video filter type." );
        return;
    }
    module_Put( p_obj );

    psz_string = config_GetPsz( p_intf, psz_filter_type );

    if( !psz_string ) psz_string = strdup( "" );

    psz_parser = strstr( psz_string, psz_name );

    if( b_add )
    {
        if( !psz_parser )
        {
            psz_parser = psz_string;
            if( asprintf( &psz_string, ( *psz_string ) ? "%s:%s" : "%s%s",
                            psz_string, psz_name ) == -1 )
                return;
            free( psz_parser );
        }
        else
        {
            return;
        }
    }
    else
    {
        if( psz_parser )
        {
            if( *( psz_parser + strlen( psz_name ) ) == ':' )
            {
                memmove( psz_parser, psz_parser + strlen( psz_name ) + 1,
                         strlen( psz_parser + strlen( psz_name ) + 1 ) + 1 );
            }
            else
            {
                *psz_parser = '\0';
            }

            /* Remove trailing : : */
            if( strlen( psz_string ) > 0 &&
                *( psz_string + strlen( psz_string ) -1 ) == ':' )
            {
                *( psz_string + strlen( psz_string ) -1 ) = '\0';
            }
        }
        else
        {
            free( psz_string );
            return;
        }
    }
    /* Vout is not kept, so put that in the config */
    config_PutPsz( p_intf, psz_filter_type, psz_string );
    if( !strcmp( psz_filter_type, "video-filter" ) )
        ui.videoFilterText->setText( psz_string );
    else if( !strcmp( psz_filter_type, "vout-filter" ) )
        ui.voutFilterText->setText( psz_string );
    else if( !strcmp( psz_filter_type, "sub-filter" ) )
        ui.subpictureFilterText->setText( psz_string );

    /* Try to set on the fly */
    p_vout = ( vout_thread_t * )vlc_object_find( p_intf, VLC_OBJECT_VOUT,
                                              FIND_ANYWHERE );
    if( p_vout )
    {
        if( !strcmp( psz_filter_type, "sub-filter" ) )
            var_SetString( p_vout->p_spu, psz_filter_type, psz_string );
        else
            var_SetString( p_vout, psz_filter_type, psz_string );
        vlc_object_release( p_vout );
    }

    free( psz_string );
}

void ExtVideo::updateFilters()
{
    QString module = ModuleFromWidgetName( sender() );
    //std::cout << "Module name: " << module.toStdString() << std::endl;

    QCheckBox *checkbox = qobject_cast<QCheckBox*>( sender() );
    QGroupBox *groupbox = qobject_cast<QGroupBox*>( sender() );

    ChangeVFiltersString( qtu( module ),
                          checkbox ? checkbox->isChecked()
                                   : groupbox->isChecked() );
}

void ExtVideo::initComboBoxItems( QObject *widget )
{
    QComboBox *combobox = qobject_cast<QComboBox*>( widget );
    if( !combobox ) return;
    QString option = OptionFromWidgetName( widget );
    module_config_t *p_item = config_FindConfig( VLC_OBJECT( p_intf ),
                                                 qtu( option ) );
    if( p_item )
    {
        int i_type = p_item->i_type & CONFIG_ITEM;
        for( int i_index = 0; i_index < p_item->i_list; i_index++ )
        {
            if( i_type == CONFIG_ITEM_INTEGER
             || i_type == CONFIG_ITEM_BOOL )
                combobox->addItem( qfu( p_item->ppsz_list_text[i_index] ),
                                   p_item->pi_list[i_index] );
            else if( i_type == CONFIG_ITEM_STRING )
                combobox->addItem( qfu( p_item->ppsz_list_text[i_index] ),
                                   p_item->ppsz_list[i_index] );
        }
    }
    else
    {
        msg_Err( p_intf, "Couldn't find option \"%s\".",
                 qtu( option ) );
    }
}

void ExtVideo::setWidgetValue( QObject *widget )
{
    QString module = ModuleFromWidgetName( widget->parent() );
    //std::cout << "Module name: " << module.toStdString() << std::endl;
    QString option = OptionFromWidgetName( widget );
    //std::cout << "Option name: " << option.toStdString() << std::endl;

    vlc_object_t *p_obj = ( vlc_object_t * )
        vlc_object_find_name( p_intf->p_libvlc,
                              qtu( module ),
                              FIND_CHILD );
    int i_type;
    vlc_value_t val;

    if( !p_obj )
    {
#if 0
        msg_Dbg( p_intf,
                 "Module instance %s not found, looking in config values.",
                 qtu( module ) );
#endif
        i_type = config_GetType( p_intf, qtu( option ) ) & 0xf0;
        switch( i_type )
        {
            case VLC_VAR_INTEGER:
            case VLC_VAR_BOOL:
                val.i_int = config_GetInt( p_intf, qtu( option ) );
                break;
            case VLC_VAR_FLOAT:
                val.f_float = config_GetFloat( p_intf, qtu( option ) );
                break;
            case VLC_VAR_STRING:
                val.psz_string = config_GetPsz( p_intf, qtu( option ) );
                break;
        }
    }
    else
    {
        i_type = var_Type( p_obj, qtu( option ) ) & 0xf0;
        var_Get( p_obj, qtu( option ), &val );
        vlc_object_release( p_obj );
    }

    /* Try to cast to all the widgets we're likely to encounter. Only
     * one of the casts is expected to work. */
    QSlider        *slider        = qobject_cast<QSlider*>       ( widget );
    QCheckBox      *checkbox      = qobject_cast<QCheckBox*>     ( widget );
    QSpinBox       *spinbox       = qobject_cast<QSpinBox*>      ( widget );
    QDoubleSpinBox *doublespinbox = qobject_cast<QDoubleSpinBox*>( widget );
    QDial          *dial          = qobject_cast<QDial*>         ( widget );
    QLineEdit      *lineedit      = qobject_cast<QLineEdit*>     ( widget );
    QComboBox      *combobox      = qobject_cast<QComboBox*>     ( widget );

    if( i_type == VLC_VAR_INTEGER || i_type == VLC_VAR_BOOL )
    {
        int i_int = 0;
        if( slider )        slider->setValue( val.i_int );
        else if( checkbox ) checkbox->setCheckState( val.i_int? Qt::Checked
                                                              : Qt::Unchecked );
        else if( spinbox )  spinbox->setValue( val.i_int );
        else if( dial )     dial->setValue( ( 540-val.i_int )%360 );
        else if( lineedit )
        {
            char str[30];
            sprintf( str, "%06X", val.i_int );
            lineedit->setText( str );
        }
        else if( combobox ) combobox->setCurrentIndex(
                            combobox->findData( val.i_int ) );
        else msg_Warn( p_intf, "Oops %s %s %d", __FILE__, __func__, __LINE__ );
    }
    else if( i_type == VLC_VAR_FLOAT )
    {
        double f_float = 0;
        if( slider ) slider->setValue( ( int )( val.f_float*( double )slider->tickInterval() ) ); /* hack alert! */
        else if( doublespinbox ) doublespinbox->setValue( val.f_float );
        else msg_Warn( p_intf, "Oops %s %s %d", __FILE__, __func__, __LINE__ );
    }
    else if( i_type == VLC_VAR_STRING )
    {
        if( lineedit ) lineedit->setText( qfu( val.psz_string ) );
        else if( combobox ) combobox->setCurrentIndex(
                            combobox->findData( qfu( val.psz_string ) ) );
        else msg_Warn( p_intf, "Oops %s %s %d", __FILE__, __func__, __LINE__ );
        free( val.psz_string );
    }
    else
        msg_Err( p_intf,
                 "Module %s's %s variable is of an unsupported type ( %d )",
                 qtu( module ),
                 qtu( option ),
                 i_type );
}

void ExtVideo::updateFilterOptions()
{
    QString module = ModuleFromWidgetName( sender()->parent() );
    //std::cout << "Module name: " << module.toStdString() << std::endl;
    QString option = OptionFromWidgetName( sender() );
    //std::cout << "Option name: " << option.toStdString() << std::endl;

    vlc_object_t *p_obj = ( vlc_object_t * )
        vlc_object_find_name( p_intf->p_libvlc,
                              qtu( module ),
                              FIND_CHILD );
    if( !p_obj )
    {
        msg_Err( p_intf, "Module %s not found.", qtu( module ) );
        return;
    }

    int i_type = var_Type( p_obj, qtu( option ) );
    bool b_is_command = ( i_type & VLC_VAR_ISCOMMAND );
    if( !b_is_command )
    {
        msg_Warn( p_intf, "Module %s's %s variable isn't a command. You'll need to restart the filter to take change into account.",
                 qtu( module ),
                 qtu( option ) );
        /* FIXME: restart automatically somewhere near the end of this function */
    }

    /* Try to cast to all the widgets we're likely to encounter. Only
     * one of the casts is expected to work. */
    QSlider        *slider        = qobject_cast<QSlider*>       ( sender() );
    QCheckBox      *checkbox      = qobject_cast<QCheckBox*>     ( sender() );
    QSpinBox       *spinbox       = qobject_cast<QSpinBox*>      ( sender() );
    QDoubleSpinBox *doublespinbox = qobject_cast<QDoubleSpinBox*>( sender() );
    QDial          *dial          = qobject_cast<QDial*>         ( sender() );
    QLineEdit      *lineedit      = qobject_cast<QLineEdit*>     ( sender() );
    QComboBox      *combobox      = qobject_cast<QComboBox*>     ( sender() );

    i_type &= 0xf0;
    if( i_type == VLC_VAR_INTEGER || i_type == VLC_VAR_BOOL )
    {
        int i_int = 0;
        if( slider )        i_int = slider->value();
        else if( checkbox ) i_int = checkbox->checkState() == Qt::Checked;
        else if( spinbox )  i_int = spinbox->value();
        else if( dial )     i_int = ( 540-dial->value() )%360;
        else if( lineedit ) i_int = lineedit->text().toInt( NULL,16 );
        else if( combobox ) i_int = combobox->itemData( combobox->currentIndex() ).toInt();
        else msg_Warn( p_intf, "Oops %s %s %d", __FILE__, __func__, __LINE__ );
        config_PutInt( p_intf, qtu( option ), i_int );
        if( b_is_command )
        {
            if( i_type == VLC_VAR_INTEGER )
                var_SetInteger( p_obj, qtu( option ), i_int );
            else
                var_SetBool( p_obj, qtu( option ), i_int );
        }
    }
    else if( i_type == VLC_VAR_FLOAT )
    {
        double f_float = 0;
        if( slider )             f_float = ( double )slider->value()
                                         / ( double )slider->tickInterval(); /* hack alert! */
        else if( doublespinbox ) f_float = doublespinbox->value();
        else if( lineedit ) f_float = lineedit->text().toDouble();
        else msg_Warn( p_intf, "Oops %s %s %d", __FILE__, __func__, __LINE__ );
        config_PutFloat( p_intf, qtu( option ), f_float );
        if( b_is_command )
            var_SetFloat( p_obj, qtu( option ), f_float );
    }
    else if( i_type == VLC_VAR_STRING )
    {
        char *psz_string = NULL;
        if( lineedit ) psz_string = strdup( qtu( lineedit->text() ) );
        else if( combobox ) psz_string = strdup( qtu( combobox->itemData(
                                       combobox->currentIndex() ).toString() ) );
        else msg_Warn( p_intf, "Oops %s %s %d", __FILE__, __func__, __LINE__ );
        config_PutPsz( p_intf, qtu( option ), psz_string );
        if( b_is_command )
            var_SetString( p_obj, qtu( option ), psz_string );
        free( psz_string );
    }
    else
        msg_Err( p_intf,
                 "Module %s's %s variable is of an unsupported type ( %d )",
                 qtu( module ),
                 qtu( option ),
                 i_type );

    vlc_object_release( p_obj );
}

#if 0
void ExtVideo::gotoConf( QObject* src )
{
#define SHOWCONF( module ) \
    if( src->objectName().contains( module ) ) \
    { \
        PrefsDialog::getInstance( p_intf )->showModulePrefs( module ); \
        return; \
    }
    SHOWCONF( "clone" );
    SHOWCONF( "magnify" );
    SHOWCONF( "wave" );
    SHOWCONF( "ripple" );
    SHOWCONF( "invert" );
    SHOWCONF( "puzzle" );
    SHOWCONF( "wall" );
    SHOWCONF( "gradient" );
    SHOWCONF( "colorthres" )
}
#endif

/**********************************************************************
 * v4l2 controls
 **********************************************************************/

ExtV4l2::ExtV4l2( intf_thread_t *_p_intf, QWidget *_parent )
    : QWidget( _parent ), p_intf( _p_intf )
{
    ui.setupUi( this );

    BUTTONACT( ui.refresh, Refresh() );

    box = NULL;
}

ExtV4l2::~ExtV4l2()
{
    if( box )
        delete box;
}

void ExtV4l2::showEvent( QShowEvent *event )
{
    QWidget::showEvent( event );
    Refresh();
}

void ExtV4l2::Refresh( void )
{
    vlc_object_t *p_obj = (vlc_object_t*)vlc_object_find_name( p_intf, "v4l2", FIND_ANYWHERE );
    ui.help->hide();
    if( box )
    {
        ui.vboxLayout->removeWidget( box );
        delete box;
        box = NULL;
    }
    if( p_obj )
    {
        vlc_value_t val, text, name;
        int i_ret = var_Change( p_obj, "controls", VLC_VAR_GETCHOICES,
                                &val, &text );
        if( i_ret < 0 )
        {
            msg_Err( p_intf, "Oops, v4l2 object doesn't have a 'controls' variable." );
            ui.help->show();
            vlc_object_release( p_obj );
            return;
        }

        box = new QGroupBox( this );
        ui.vboxLayout->addWidget( box );
        QVBoxLayout *layout = new QVBoxLayout( box );
        box->setLayout( layout );

        for( int i = 0; i < val.p_list->i_count; i++ )
        {
            const char *psz_var = text.p_list->p_values[i].psz_string;
            var_Change( p_obj, psz_var, VLC_VAR_GETTEXT, &name, NULL );
            const char *psz_label = name.psz_string;
            msg_Dbg( p_intf, "v4l2 control \"%x\": %s (%s)",
                     val.p_list->p_values[i].i_int, psz_var, name.psz_string );

            int i_type = var_Type( p_obj, psz_var );
            switch( i_type & VLC_VAR_TYPE )
            {
                case VLC_VAR_INTEGER:
                {
                    QLabel *label = new QLabel( psz_label, box );
                    QHBoxLayout *hlayout = new QHBoxLayout();
                    hlayout->addWidget( label );
                    int i_val = var_GetInteger( p_obj, psz_var );
                    if( i_type & VLC_VAR_HASCHOICE )
                    {
                        QComboBox *combobox = new QComboBox( box );
                        combobox->setObjectName( psz_var );

                        vlc_value_t val2, text2;
                        var_Change( p_obj, psz_var, VLC_VAR_GETCHOICES,
                                    &val2, &text2 );
                        for( int j = 0; j < val2.p_list->i_count; j++ )
                        {
                            combobox->addItem(
                                       text2.p_list->p_values[j].psz_string,
                                       val2.p_list->p_values[j].i_int );
                            if( i_val == val2.p_list->p_values[j].i_int )
                                combobox->setCurrentIndex( j );
                        }
                        var_Change( p_obj, psz_var, VLC_VAR_FREELIST,
                                    &val2, &text2 );

                        CONNECT( combobox, currentIndexChanged( int ), this,
                                 ValueChange( int ) );
                        hlayout->addWidget( combobox );
                    }
                    else
                    {
                        QSlider *slider = new QSlider( box );
                        slider->setObjectName( psz_var );
                        slider->setOrientation( Qt::Horizontal );
                        vlc_value_t val2;
                        var_Change( p_obj, psz_var, VLC_VAR_GETMIN,
                                    &val2, NULL );
                        slider->setMinimum( val2.i_int );
                        var_Change( p_obj, psz_var, VLC_VAR_GETMAX,
                                    &val2, NULL );
                        slider->setMaximum( val2.i_int );
                        var_Change( p_obj, psz_var, VLC_VAR_GETSTEP,
                                    &val2, NULL );
                        slider->setSingleStep( val2.i_int );
                        slider->setValue( i_val );

                        CONNECT( slider, valueChanged( int ), this,
                                 ValueChange( int ) );
                        hlayout->addWidget( slider );
                    }
                    layout->addLayout( hlayout );
                    break;
                }
                case VLC_VAR_BOOL:
                {
                    QCheckBox *button = new QCheckBox( psz_label, box );
                    button->setObjectName( psz_var );
                    button->setChecked( var_GetBool( p_obj, psz_var ) );

                    CONNECT( button, clicked( bool ), this,
                             ValueChange( bool ) );
                    layout->addWidget( button );
                    break;
                }
                case VLC_VAR_VOID:
                {
                    if( i_type & VLC_VAR_ISCOMMAND )
                    {
                        QPushButton *button = new QPushButton( psz_label, box );
                        button->setObjectName( psz_var );

                        CONNECT( button, clicked( bool ), this,
                                 ValueChange( bool ) );
                        layout->addWidget( button );
                    }
                    else
                    {
                        QLabel *label = new QLabel( psz_label, box );
                        layout->addWidget( label );
                    }
                    break;
                }
                default:
                    msg_Warn( p_intf, "Unhandled var type for %s", psz_var );
                    break;
            }
            free( name.psz_string );
        }
        var_Change( p_obj, "controls", VLC_VAR_FREELIST, &val, &text );
        vlc_object_release( p_obj );
    }
    else
    {
        msg_Dbg( p_intf, "Couldn't find v4l2 instance" );
        ui.help->show();
    }
}

void ExtV4l2::ValueChange( bool value )
{
    ValueChange( (int)value );
}

void ExtV4l2::ValueChange( int value )
{
    QObject *s = sender();
    vlc_object_t *p_obj = (vlc_object_t*)vlc_object_find_name( p_intf, "v4l2", FIND_ANYWHERE );
    if( p_obj )
    {
        char *psz_var = strdup( qtu( s->objectName() ) );
        int i_type = var_Type( p_obj, psz_var );
        switch( i_type & VLC_VAR_TYPE )
        {
            case VLC_VAR_INTEGER:
                if( i_type & VLC_VAR_HASCHOICE )
                {
                    QComboBox *combobox = qobject_cast<QComboBox*>( s );
                    value = combobox->itemData( value ).toInt();
                }
                var_SetInteger( p_obj, psz_var, value );
                break;
            case VLC_VAR_BOOL:
                var_SetBool( p_obj, psz_var, value );
                break;
            case VLC_VAR_VOID:
                var_SetVoid( p_obj, psz_var );
                break;
        }
        free( psz_var );
        vlc_object_release( p_obj );
    }
    else
    {
        msg_Warn( p_intf, "Oops, v4l2 object isn't available anymore" );
        Refresh();
    }
}

/**********************************************************************
 * Equalizer
 **********************************************************************/

static const QString band_frequencies[] =
{
    "  60 Hz  ", " 170 Hz ", " 310 Hz ", " 600 Hz ", "  1 kHz ",
    "  3 kHz  ", "  6 kHz ", " 12 kHz ", " 14 kHz ", " 16 kHz "
};

Equalizer::Equalizer( intf_thread_t *_p_intf, QWidget *_parent ) :
                            QWidget( _parent ) , p_intf( _p_intf )
{
    QFont smallFont = QApplication::font( static_cast<QWidget*>( 0 ) );
    smallFont.setPointSize( smallFont.pointSize() - 3 );

    ui.setupUi( this );
    presetsComboBox = ui.presetsCombo;

    ui.preampLabel->setFont( smallFont );
    ui.preampSlider->setMaximum( 400 );
    for( int i = 0 ; i < NB_PRESETS ; i ++ )
    {
        ui.presetsCombo->addItem( qtr( preset_list_text[i] ),
                                  QVariant( i ) );
    }
    CONNECT( ui.presetsCombo, activated( int ), this, setPreset( int ) );

    BUTTONACT( ui.enableCheck, enable() );
    BUTTONACT( ui.eq2PassCheck, set2Pass() );

    CONNECT( ui.preampSlider, valueChanged( int ), this, setPreamp() );

    QGridLayout *grid = new QGridLayout( ui.frame );
    grid->setMargin( 0 );
    for( int i = 0 ; i < BANDS ; i++ )
    {
        bands[i] = new QSlider( Qt::Vertical );
        bands[i]->setMaximum( 400 );
        bands[i]->setValue( 200 );
        CONNECT( bands[i], valueChanged( int ), this, setBand() );
        band_texts[i] = new QLabel( band_frequencies[i] + "\n0.0dB" );
        band_texts[i]->setFont( smallFont );
        grid->addWidget( bands[i], 0, i );
        grid->addWidget( band_texts[i], 1, i );
    }

    /* Write down initial values */
    aout_instance_t *p_aout = ( aout_instance_t * )vlc_object_find( p_intf,
                                    VLC_OBJECT_AOUT, FIND_ANYWHERE );
    char *psz_af;
    char *psz_bands;
    float f_preamp;
    if( p_aout )
    {
        psz_af = var_GetNonEmptyString( p_aout, "audio-filter" );
        if( var_GetBool( p_aout, "equalizer-2pass" ) )
            ui.eq2PassCheck->setChecked( true );
        psz_bands = var_GetNonEmptyString( p_aout, "equalizer-bands" );
        f_preamp = var_GetFloat( p_aout, "equalizer-preamp" );
        vlc_object_release( p_aout );
    }
    else
    {
        psz_af = config_GetPsz( p_intf, "audio-filter" );
        if( config_GetInt( p_intf, "equalizer-2pass" ) )
            ui.eq2PassCheck->setChecked( true );
        psz_bands = config_GetPsz( p_intf, "equalizer-bands" );
        f_preamp = config_GetFloat( p_intf, "equalizer-preamp" );
    }
    if( psz_af && strstr( psz_af, "equalizer" ) != NULL )
        ui.enableCheck->setChecked( true );
    free( psz_af );
    enable( ui.enableCheck->isChecked() );

    setValues( psz_bands, f_preamp );
}

Equalizer::~Equalizer()
{
}

void Equalizer::enable()
{
    bool en = ui.enableCheck->isChecked();
    aout_EnableFilter( VLC_OBJECT( p_intf ), "equalizer",
                       en ? true : false );
//    aout_EnableFilter( VLC_OBJECT( p_intf ), "upmixer",
//                       en ? true : false );
//     aout_EnableFilter( VLC_OBJECT( p_intf ), "vsurround",
//                       en ? true : false );
     enable( en );
}

void Equalizer::enable( bool en )
{
    ui.eq2PassCheck->setEnabled( en );
    ui.preampLabel->setEnabled( en );
    ui.preampSlider->setEnabled( en  );
    for( int i = 0 ; i< BANDS; i++ )
    {
        bands[i]->setEnabled( en ); band_texts[i]->setEnabled( en );
    }
}

void Equalizer::set2Pass()
{
    aout_instance_t *p_aout= ( aout_instance_t * )vlc_object_find( p_intf,
                                 VLC_OBJECT_AOUT, FIND_ANYWHERE );
    bool b_2p = ui.eq2PassCheck->isChecked();

    if( p_aout == NULL )
        config_PutInt( p_intf, "equalizer-2pass", b_2p );
    else
    {
        var_SetBool( p_aout, "equalizer-2pass", b_2p );
        config_PutInt( p_intf, "equalizer-2pass", b_2p );
        for( int i = 0; i < p_aout->i_nb_inputs; i++ )
        {
            p_aout->pp_inputs[i]->b_restart = true;
        }
        vlc_object_release( p_aout );
    }
}

void Equalizer::setPreamp()
{
    float f= ( float )(  ui.preampSlider->value() ) /10 - 20;
    char psz_val[5];
    aout_instance_t *p_aout= ( aout_instance_t * )vlc_object_find( p_intf,
                                       VLC_OBJECT_AOUT, FIND_ANYWHERE );

    sprintf( psz_val, "%.1f", f );
    ui.preampLabel->setText( qtr( "Preamp\n" ) + psz_val + qtr( "dB" ) );
    if( p_aout )
    {
        delCallbacks( p_aout );
        var_SetFloat( p_aout, "equalizer-preamp", f );
        addCallbacks( p_aout );
        vlc_object_release( p_aout );
    }
    config_PutFloat( p_intf, "equalizer-preamp", f );
}

void Equalizer::setBand()
{
    char psz_values[102]; memset( psz_values, 0, 102 );

    /**\todo smoothing */

    for( int i = 0 ; i< BANDS ; i++ )
    {
        char psz_val[8];
        float f_val = ( float )(  bands[i]->value() ) / 10 - 20 ;
        sprintf( psz_values, "%s %f", psz_values, f_val );
        sprintf( psz_val, "% 5.1f", f_val );
        band_texts[i]->setText( band_frequencies[i] + "\n" + psz_val + "dB" );
    }
    aout_instance_t *p_aout= ( aout_instance_t * )vlc_object_find( p_intf,
                                          VLC_OBJECT_AOUT, FIND_ANYWHERE );
    if( p_aout )
    {
        delCallbacks( p_aout );
        var_SetString( p_aout, "equalizer-bands", psz_values );
        addCallbacks( p_aout );
        vlc_object_release( p_aout );
    }
}
void Equalizer::setValues( char *psz_bands, float f_preamp )
{
    char *p = psz_bands;
    if ( p )
    {
        for( int i = 0; i < BANDS; i++ )
        {
            char psz_val[8];
            float f = strtof( p, &p );
            int  i_val= ( int )( ( f + 20 ) * 10 );
            bands[i]->setValue(  i_val );
            sprintf( psz_val, "% 5.1f", f );
            band_texts[i]->setText( band_frequencies[i] + "\n" + psz_val +
                                    "dB" );
            if( p == NULL || *p == '\0' ) break;
            p++;
            if( *p == '\0' )  break;
        }
    }
    char psz_val[5];
    int i_val = ( int )( ( f_preamp + 20 ) * 10 );
    sprintf( psz_val, "%.1f", f_preamp );
    ui.preampSlider->setValue( i_val );
    ui.preampLabel->setText( qtr( "Preamp\n" ) + psz_val + qtr( "dB" ) );
}

void Equalizer::setPreset( int preset )
{
    aout_instance_t *p_aout= ( aout_instance_t * )vlc_object_find( p_intf,
                                                VLC_OBJECT_AOUT, FIND_ANYWHERE );

    char psz_values[102]; memset( psz_values, 0, 102 );
    char psz_values2[102];memset( psz_values2, 0, 102 );
    for( int i = 0 ; i< BANDS ;i++ )
    {
        strcpy( psz_values2, psz_values );

        sprintf( psz_values, "%s %5.1f",
                 psz_values2, eqz_preset_10b[preset]->f_amp[i] );
    }

    if( p_aout )
    {
        delCallbacks( p_aout );
        var_SetString( p_aout, "equalizer-bands", psz_values );
        var_SetFloat( p_aout, "equalizer-preamp",
                      eqz_preset_10b[preset]->f_preamp );
        addCallbacks( p_aout );
        vlc_object_release( p_aout );
    }
    config_PutPsz( p_intf, "equalizer-bands", psz_values );
    config_PutFloat( p_intf, "equalizer-preamp",
                    eqz_preset_10b[preset]->f_preamp );

    setValues( psz_values, eqz_preset_10b[preset]->f_preamp );
}

static int PresetCallback( vlc_object_t *p_this, char const *psz_cmd,
                         vlc_value_t oldval, vlc_value_t newval, void *p_data )
{
    char *psz_preset = newval.psz_string;
    Equalizer *eq = ( Equalizer * )p_data;
    eq->presetsComboBox->setCurrentIndex( eq->presetsComboBox->findText( qfu( psz_preset ) ) );
    return VLC_SUCCESS;
}

void Equalizer::delCallbacks( aout_instance_t *p_aout )
{
    //var_DelCallback( p_aout, "equalizer-bands", EqzCallback, this );
    //var_DelCallback( p_aout, "equalizer-preamp", EqzCallback, this );
    var_DelCallback( p_aout, "equalizer-preset", PresetCallback, this );
}

void Equalizer::addCallbacks( aout_instance_t *p_aout )
{
    //var_AddCallback( p_aout, "equalizer-bands", EqzCallback, this );
    //var_AddCallback( p_aout, "equalizer-preamp", EqzCallback, this );
    var_AddCallback( p_aout, "equalizer-preset", PresetCallback, this );
}

/**********************************************************************
 * Audio filters
 **********************************************************************/

/**********************************************************************
 * Spatializer
 **********************************************************************/
static const char *psz_control_names[] =
{
    "Roomsize", "Width" , "Wet", "Dry", "Damp"
};

Spatializer::Spatializer( intf_thread_t *_p_intf, QWidget *_parent ) :
    QWidget( _parent ) , p_intf( _p_intf )
{
    QFont smallFont = QApplication::font( static_cast<QWidget*>( 0 ) );
    smallFont.setPointSize( smallFont.pointSize() - 3 );

    QGridLayout *layout = new QGridLayout( this );
    layout->setMargin( 0 );

    enableCheck = new QCheckBox( qfu( "Enable spatializer" ) );
    layout->addWidget( enableCheck, 0, 0, 1, NUM_SP_CTRL );

    for( int i = 0 ; i < NUM_SP_CTRL ; i++ )
    {
        spatCtrl[i] = new QSlider( Qt::Vertical );
        if( i < 2 )
        {
            spatCtrl[i]->setMaximum( 10 );
            spatCtrl[i]->setValue( 2 );
        }
        else
        {
            spatCtrl[i]->setMaximum( 10 );
            spatCtrl[i]->setValue( 0 );
            spatCtrl[i]->setMinimum( -10 );
        }
        oldControlVars[i] = spatCtrl[i]->value();
        CONNECT( spatCtrl[i], valueChanged( int ), this, setInitValues() );
        ctrl_texts[i] = new QLabel( qfu( psz_control_names[i] ) + "\n" );
        ctrl_texts[i]->setFont( smallFont );
        ctrl_readout[i] = new QLabel( "" );
        ctrl_readout[i]->setFont( smallFont );
        layout->addWidget( spatCtrl[i], 1, i );
        layout->addWidget( ctrl_readout[i], 2, i );
        layout->addWidget( ctrl_texts[i], 3, i );
    }

    BUTTONACT( enableCheck, enable() );

    /* Write down initial values */
    aout_instance_t *p_aout = ( aout_instance_t * )
        vlc_object_find( p_intf, VLC_OBJECT_AOUT, FIND_ANYWHERE );
    char *psz_af;

    if( p_aout )
    {
        psz_af = var_GetNonEmptyString( p_aout, "audio-filter" );
        for( int i = 0; i < NUM_SP_CTRL ; i++ )
        {
            controlVars[i] = var_GetFloat( p_aout, psz_control_names[i] );
        }
        vlc_object_release( p_aout );
    }
    else
    {
        psz_af = config_GetPsz( p_intf, "audio-filter" );
        for( int i = 0; i < NUM_SP_CTRL ; i++ )
        {
            controlVars[i] = config_GetFloat( p_intf, psz_control_names[i] );
        }
    }
    if( psz_af && strstr( psz_af, "spatializer" ) != NULL )
        enableCheck->setChecked( true );
    free( psz_af );
    enable( enableCheck->isChecked() );
    setValues( controlVars );
}

Spatializer::~Spatializer()
{
}

void Spatializer::enable()
{
    bool en = enableCheck->isChecked();
    aout_EnableFilter( VLC_OBJECT( p_intf ), "spatializer",
            en ? true : false );
    enable( en );
}

void Spatializer::enable( bool en )
{
    for( int i = 0 ; i< NUM_SP_CTRL; i++ )
    {
        spatCtrl[i]->setEnabled( en );
        ctrl_texts[i]->setEnabled( en );
        ctrl_readout[i]->setEnabled( en );
    }
}
void Spatializer::setInitValues()
{
    setValues( controlVars );
}

void Spatializer::setValues( float *controlVars )
{
    char psz_val[5];
    char var_name[5];
    aout_instance_t *p_aout= ( aout_instance_t * )
        vlc_object_find( p_intf, VLC_OBJECT_AOUT, FIND_ANYWHERE );

    for( int i = 0 ; i < NUM_SP_CTRL ; i++ )
    {
        float f= ( float )(  spatCtrl[i]->value() );
        sprintf( psz_val, "%.1f", f );
        ctrl_readout[i]->setText( psz_val );
    }
    if( p_aout )
    {
        for( int i = 0 ; i < NUM_SP_CTRL ; i++ )
        {
            if( oldControlVars[i] != spatCtrl[i]->value() )
            {
                var_SetFloat( p_aout, psz_control_names[i],
                        ( float )spatCtrl[i]->value() );
                config_PutFloat( p_intf, psz_control_names[i],
                        ( float ) spatCtrl[i]->value() );
                oldControlVars[i] = ( float ) spatCtrl[i]->value();
            }
        }
        vlc_object_release( p_aout );
    }

}
void Spatializer::delCallbacks( aout_instance_t *p_aout )
{
    //    var_DelCallback( p_aout, "Spatializer-bands", EqzCallback, this );
    //    var_DelCallback( p_aout, "Spatializer-preamp", EqzCallback, this );
}

void Spatializer::addCallbacks( aout_instance_t *p_aout )
{
    //    var_AddCallback( p_aout, "Spatializer-bands", EqzCallback, this );
    //    var_AddCallback( p_aout, "Spatializer-preamp", EqzCallback, this );
}

#include <QToolButton>
#include <QGridLayout>

SyncControls::SyncControls( intf_thread_t *_p_intf, QWidget *_parent ) :
                            QWidget( _parent ) , p_intf( _p_intf )
{
    QGroupBox *AVBox, *subsBox;

    QToolButton *moinsAV, *plusAV;
    QToolButton *moinssubs, *plussubs;
    QToolButton *moinssubSpeed, *plussubSpeed;

    QToolButton *updateButton;

    QGridLayout *mainLayout = new QGridLayout( this );

    /* AV sync */
    AVBox = new QGroupBox( qtr( "Audio/Video" ) );
    QGridLayout *AVLayout = new QGridLayout( AVBox );

    moinsAV = new QToolButton;
    moinsAV->setToolButtonStyle( Qt::ToolButtonTextOnly );
    moinsAV->setAutoRaise( true );
    moinsAV->setText( "-" );
    AVLayout->addWidget( moinsAV, 0, 1, 1, 1 );

    plusAV = new QToolButton;
    plusAV->setToolButtonStyle( Qt::ToolButtonTextOnly );
    plusAV->setAutoRaise( true );
    plusAV->setText( "+" );
    AVLayout->addWidget( plusAV, 0, 3, 1, 1 );

    QLabel *AVLabel = new QLabel;
    AVLabel->setText( qtr( "Advance of audio over video:" ) );
    AVLayout->addWidget( AVLabel, 0, 0, 1, 1 );

    AVSpin = new QDoubleSpinBox;
    AVSpin->setAlignment( Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter );
    AVSpin->setDecimals( 3 );
    AVSpin->setMinimum( -100 );
    AVSpin->setMaximum( 100 );
    AVSpin->setSingleStep( 0.1 );
    AVSpin->setToolTip( qtr( "A positive value means that\n"
                             "the audio is ahead of the video" ) );
    AVSpin->setSuffix( "s" );
    AVLayout->addWidget( AVSpin, 0, 2, 1, 1 );
    mainLayout->addWidget( AVBox, 1, 0, 1, 5 );


    /* Subs */
    subsBox = new QGroupBox( qtr( "Subtitles/Video" ) );
    QGridLayout *subsLayout = new QGridLayout( subsBox );

    moinssubs = new QToolButton;
    moinssubs->setToolButtonStyle( Qt::ToolButtonTextOnly );
    moinssubs->setAutoRaise( true );
    moinssubs->setText( "-" );
    subsLayout->addWidget( moinssubs, 0, 1, 1, 1 );

    plussubs = new QToolButton;
    plussubs->setToolButtonStyle( Qt::ToolButtonTextOnly );
    plussubs->setAutoRaise( true );
    plussubs->setText( "+" );
    subsLayout->addWidget( plussubs, 0, 3, 1, 1 );

    QLabel *subsLabel = new QLabel;
    subsLabel->setText( qtr( "Advance of subtitles over video:" ) );
    subsLayout->addWidget( subsLabel, 0, 0, 1, 1 );

    subsSpin = new QDoubleSpinBox;
    subsSpin->setAlignment( Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter );
    subsSpin->setDecimals( 3 );
    subsSpin->setMinimum( -100 );
    subsSpin->setMaximum( 100 );
    subsSpin->setSingleStep( 0.1 );
    subsSpin->setToolTip( qtr( "A positive value means that\n"
                             "the subtitles are ahead of the video" ) );
    subsSpin->setSuffix( "s" );
    subsLayout->addWidget( subsSpin, 0, 2, 1, 1 );


    moinssubSpeed = new QToolButton;
    moinssubSpeed->setToolButtonStyle( Qt::ToolButtonTextOnly );
    moinssubSpeed->setAutoRaise( true );
    moinssubSpeed->setText( "-" );
    subsLayout->addWidget( moinssubSpeed, 1, 1, 1, 1 );

    plussubSpeed = new QToolButton;
    plussubSpeed->setToolButtonStyle( Qt::ToolButtonTextOnly );
    plussubSpeed->setAutoRaise( true );
    plussubSpeed->setText( "+" );
    subsLayout->addWidget( plussubSpeed, 1, 3, 1, 1 );

    QLabel *subSpeedLabel = new QLabel;
    subSpeedLabel->setText( qtr( "Speed of the subtitles:" ) );
    subsLayout->addWidget( subSpeedLabel, 1, 0, 1, 3 );

    subSpeedSpin = new QDoubleSpinBox;
    subSpeedSpin->setAlignment( Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter );
    subSpeedSpin->setDecimals( 3 );
    subSpeedSpin->setMinimum( 1 );
    subSpeedSpin->setMaximum( 100 );
    subSpeedSpin->setSingleStep( 0.2 );
    subsLayout->addWidget( subSpeedSpin, 1, 2, 1, 1 );

    mainLayout->addWidget( subsBox, 2, 0, 2, 5 );

    updateButton = new QToolButton;
    updateButton->setAutoRaise( true );
    updateButton->setText( "u" );
    updateButton->setToolTip( qtr( "Force update of the values in this dialog" ) );
    mainLayout->addWidget( updateButton, 0, 4, 1, 1 );


    /* Various Connects */
    CONNECT( moinsAV, clicked(), AVSpin, stepDown () );
    CONNECT( plusAV, clicked(), AVSpin, stepUp () );
    CONNECT( moinssubs, clicked(), subsSpin, stepDown () );
    CONNECT( plussubs, clicked(), subsSpin, stepUp () );
    CONNECT( moinssubSpeed, clicked(), subSpeedSpin, stepDown () );
    CONNECT( plussubSpeed, clicked(), subSpeedSpin, stepUp () );
    CONNECT( AVSpin, valueChanged ( double ), this, advanceAudio( double ) ) ;
    CONNECT( subsSpin, valueChanged ( double ), this, advanceSubs( double ) ) ;
    CONNECT( subSpeedSpin, valueChanged ( double ),
             this, adjustSubsSpeed( double ) );
    BUTTONACT( updateButton, update() );

    /* Set it */
    update();
}

void SyncControls::update()
{
    int64_t i_delay;
    if( THEMIM->getInput() )
    {
        i_delay = var_GetTime( THEMIM->getInput(), "spu-delay" );
        AVSpin->setValue( ( (double)i_delay ) / 1000000 );
        i_delay = var_GetTime( THEMIM->getInput(), "audio-delay" );
        subsSpin->setValue( ( (double)i_delay ) / 1000000 );
        subSpeedSpin->setValue( var_GetFloat( THEMIM->getInput(), "sub-fps" ) );
    }
}

void SyncControls::advanceAudio( double f_advance )
{
    if( THEMIM->getInput() )
    {
        int64_t i_delay = var_GetTime( THEMIM->getInput(), "audio-delay" );
        i_delay += f_advance * 1000000;
        var_SetTime( THEMIM->getInput(), "audio-delay", i_delay );
        msg_Dbg( p_intf, "I am advancing Audio %d", f_advance );
    }
}

void SyncControls::advanceSubs( double f_advance )
{
    if( THEMIM->getInput() )
    {
        int64_t i_delay = var_GetTime( THEMIM->getInput(), "spu-delay" );
        i_delay += f_advance * 1000000;
        var_SetTime( THEMIM->getInput(), "spu-delay", i_delay );
        msg_Dbg( p_intf, "I am advancing subtitles %d", f_advance );
    }
}

void SyncControls::adjustSubsSpeed( double f_fps )
{
    if( THEMIM->getInput() )
    {
        var_SetFloat( THEMIM->getInput(), "sub-fps", f_fps );
    }
}

/**********************************************************************
 * Video filters / Adjust
 **********************************************************************/

/**********************************************************************
 * Extended playbak controls
 **********************************************************************/
