#include "sgui.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>


static void fail( const char* message )
{
    fputs( message, stderr );
    exit( EXIT_FAILURE );
}


#define BADPTR ((void*)0xC0FEBABE)


static void add_items( sgui_model* model, sgui_item* parent, int level,
                       int withicon )
{
    unsigned int i, j;
    sgui_item* item;
    char buffer[32];

    for( j=0; j<20; ++j )
    {
        if( sgui_model_item_children_count( model, parent )!=j )
            fail( "[add_items] model children count wrong\n" );
        if( !(item=sgui_simple_model_add_item( model, parent )) )
            fail( "[add_items] add item to model\n" );
        if( sgui_model_item_children_count( model, parent )!=(j+1) )
            fail( "[add_items] model children count wrong\n" );
        if( sgui_model_item_children_count( model, item ) )
            fail( "[add_items] item children count wrong\n" );

        if( !level )
        {
            add_items( model, item, 1, withicon );

            if( sgui_model_item_children_count( model, item )!=20 )
                fail( "[add_items] item children count wrong after add\n" );
        }

        for( i=0; i<100; ++i )
        {
            if( sgui_item_text( model, item, i ) )
                fail( "[add_items] empty item has text set\n" );
            if( sgui_item_icon( model, item, i ) )
                fail( "[add_items] empty item has icon set\n" );
        }
        for( i=0; i<100; ++i )
        {
            if( level )
                sprintf( buffer, "%d.%d.%d", j, i, level );
            else
                sprintf( buffer, "%d.%d", j, i );

            sgui_simple_item_set_icon( model, item, i, (sgui_icon*)BADPTR+i );
            sgui_simple_item_set_text( model, item, i, buffer );
        }
        for( i=0; i<3; ++i )
        {
            if( level )
                sprintf( buffer, "%d.%d.%d", j, i, level );
            else
                sprintf( buffer, "%d.%d", j, i );

            if( withicon )
            {
                if( sgui_item_icon( model, item, i )!=((sgui_icon*)BADPTR+i) )
                    fail( "[add_items] wrong icon set\n" );
            }
            else
            {
                if( sgui_item_icon( model, item, i ) )
                    fail( "[add_items] icon set without cache\n" );
            }
            if( strcmp( sgui_item_text( model, item, i ), buffer ) )
                fail( "[add_items] wrong text set\n" );
        }
        for( ; i<100; ++i )
        {
            if( sgui_item_icon( model, item, i ) )
                fail( "[add_items] icon set in non-existing column\n" );
            if( sgui_item_text( model, item, i ) )
                fail( "[add_items] text set in non-existing column\n" );
        }
    }
}

static void check_items( sgui_model* model, sgui_item* parent, int level,
                         int withicon )
{
    const sgui_item* list;
    unsigned int i, j, k;
    char buffer[32];

    for( i=0; i<20; ++i )
    {
        if( !(list = sgui_model_query_items( model, parent, i, 20-i )) )
            fail( "[check_items] could not query item sublist\n" );

        for( j=i; j<20; ++j, list=list->next )
        {
            if( !level )
            {
                check_items( model, (sgui_item*)list, 1, withicon );
                if( sgui_model_item_children_count( model, list )!=20 )
                    fail( "[check_items] item children count wrong\n" );
            }
            else
            {
                if( sgui_model_item_children_count( model, list )!=0 )
                    fail( "[check_items] item children count wrong\n" );
            }

            for( k=0; k<3; ++k )
            {
                if( level )
                    sprintf( buffer, "%d.%d.%d", 19-j, k, level );
                else
                    sprintf( buffer, "%d.%d", 19-j, k );

                if( withicon )
                {
                    if( sgui_item_icon(model,list,k)!=((sgui_icon*)BADPTR+k) )
                        fail( "[check_items] wrong icon set\n" );
                }
                else
                {
                    if( sgui_item_icon( model, list, k ) )
                        fail( "[check_items] icon set without cache\n" );
                }

                if( strcmp( sgui_item_text( model, list, k ), buffer ) )
                    fail( "[check_items] wrong text set\n" );
            }
            for( ; k<100; ++k )
            {
                if( sgui_item_icon( model, list, k ) )
                    fail( "[check_items] icon set in non-existing column\n" );
                if( sgui_item_text( model, list, k ) )
                    fail( "[check_items] text set in non-existing column\n" );
            }
        }

        if( list )
            fail( "[check_items] list end not reached yet\n" );
    }
}



int main( void )
{
    sgui_model* model;

    if( !sgui_init( ) )
        fail( "sgui_init\n" );

    /* sgui_simple_model_create */
    model = sgui_simple_model_create( 0, NULL );

    if( model )
        fail( "create simple model with 0 columns\n" );

    model = sgui_simple_model_create( 3, NULL );

    if( !model )
        fail( "create simple model with valid parameters\n" );

    /* sgui_model_get_icon_cache */
    if( sgui_model_get_icon_cache( NULL ) )
        fail( "item cache for model=NULL\n" );

    if( sgui_model_get_icon_cache( model ) )
        fail( "simple model with no icon cache returned icon cache\n" );

    /* sgui_model_column_count */
    if( sgui_model_column_count( NULL ) )
        fail( "column count for model=NULL\n" );

    if( sgui_model_column_count( model )!=3 )
        fail( "wrong column count for simple model\n" );

    /* sgui_model_item_children_count on empty and NULL model */
    if( sgui_model_item_children_count( model, NULL ) )
        fail( "children count for empty model\n" );

    if( sgui_model_item_children_count( NULL, NULL ) )
        fail( "children count for model=NULL\n" );

    if( sgui_model_item_children_count( NULL, BADPTR ) )
        fail( "children count for model=NULL, parent=BADPTR\n" );

    /* sgui_model_query_items on empty and NULL model */
    if( sgui_model_query_items( model, NULL, 0, 10 ) )
        fail( "query items for empty model returned list != NULL\n" );

    if( sgui_model_query_items( NULL, NULL, 0, 10 ) )
        fail( "query items for model=NULL returned list != NULL\n" );

    /* insert and query items */
    if( sgui_simple_model_add_item( NULL, NULL ) )
        fail( "added item to model=NULL, parent=NULL\n" );

    if( sgui_simple_model_add_item( NULL, BADPTR ) )
        fail( "added item to model=NULL, parent=BADPTR\n" );

    add_items( model, NULL, 0, 0 );
    check_items( model, NULL, 0, 0 );

    sgui_model_destroy( model );

    /* insert and query items with icons */
    model = sgui_simple_model_create( 3, BADPTR );
    add_items( model, NULL, 0, 1 );
    check_items( model, NULL, 0, 1 );
    sgui_model_destroy( model );

    /* must not segfault */
    sgui_model_query_items( NULL,   NULL,   0, 0 );
    sgui_model_query_items( BADPTR, NULL,   0, 0 );
    sgui_model_query_items( NULL,   BADPTR, 0, 0 );
    sgui_model_query_items( BADPTR, BADPTR, 0, 0 );

    sgui_model_free_item_list( NULL,   NULL   );
    sgui_model_free_item_list( BADPTR, NULL   );
    sgui_model_free_item_list( NULL,   BADPTR );

    sgui_deinit( );
    return EXIT_SUCCESS;
}

