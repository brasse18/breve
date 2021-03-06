/*****************************************************************************
 *                                                                           *
 * The breve Simulation Environment                                          *
 * Copyright (C) 2000, 2001, 2002, 2003 Jonathan Klein                       *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this program; if not, write to the Free Software               *
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA *
 *****************************************************************************/

#include "kernel.h"
#include "steve.h"
#include "expression.h"
#include "evaluation.h"
#include "world.h"
#include "shape.h"
#include "mesh.h"

#define BRSHAPEPOINTER(p)	((slShape*)BRPOINTER(p))

/*! \addtogroup InternalFunctions */
/*@{*/

int brIShapeNew( brEval args[], brEval *target, brInstance *i ) {
	target->set( new slMeshShape() );

	return EC_OK;
}

int brIAddShapeFace( brEval args[], brEval *target, brInstance *i ) {
	slShape *s = BRSHAPEPOINTER( &args[0] );
	brEvalListHead *list = BRLIST( &args[1] );
	slVector *face[ list->_vector.size()];
	std::vector< brEval >::iterator li;
	int n = 0;

	if ( !s ) return EC_OK;

	if ( list->_vector.size() < 3 ) {
		slMessage( DEBUG_ALL, "Adding a face to a shape requires a list of vectors" );
		return EC_ERROR;
	}

	for ( li = list->_vector.begin(); li != list->_vector.end(); li++ ) {
		if ( ( *li ).type() != AT_VECTOR ) {
			slFree( face );
			slMessage( DEBUG_ALL, "Adding a face to a shape requires a list of vectors" );
			return EC_ERROR;
		}

		face[ n++ ] = &BRVECTOR( &(*li) );
	}

	slAddFace( s, face, list->_vector.size() );

	return EC_OK;
}

int brIFinishShape( brEval args[], brEval *target, brInstance *i ) {
	slMeshShape *s = (slMeshShape*)BRSHAPEPOINTER( &args[0] );
	double density = BRDOUBLE( &args[1] );

	s -> finishShape( density );

	return EC_OK;
}

int brIShapeSetDensity( brEval args[], brEval *target, brInstance *i ) {
	slShape *s = BRSHAPEPOINTER( &args[0] );
	double density = BRDOUBLE( &args[1] );

	s->setDensity( density );

	return EC_OK;
}

int brIShapeSetMass( brEval args[], brEval *target, brInstance *i ) {
	slShape *s = BRSHAPEPOINTER( &args[0] );
	double mass = BRDOUBLE( &args[1] );

	s->setMass( mass );

	return EC_OK;
}

int brIShapeSetInertiaMatrix( brEval args[], brEval *target, brInstance *i ) {
	slShape *s = BRSHAPEPOINTER( &args[ 0 ] );
	s -> setInertiaMatrix( BRMATRIX( &args[ 1 ] ) );

	return EC_OK;
}

int brIMeshShapeNew( brEval args[], brEval *target, brInstance *i ) {
	char *path = brFindFile( i -> engine, BRSTRING( &args[ 0 ] ), NULL );

	if( !path ) {
		slMessage( DEBUG_ALL, "Could not locate mesh file \"%s\"", BRSTRING( &args[ 0 ] ) );
		return EC_ERROR;
	}

#ifdef HAVE_LIBASSIMP
	target -> set( slAIImport::Import( path ) );
#else
	slMessage( DEBUG_ALL, "Cannot load mesh shape -- this binary was built without libassimp" );
	return EC_ERROR;
#endif

	return EC_OK;
}

int brISphereNew( brEval args[], brEval *target, brInstance *i ) {

	target->set( new slSphere( BRDOUBLE( &args[0] ), BRDOUBLE( &args[1] ) ) );

	return EC_OK;
}

int brICubeNew( brEval args[], brEval *target, brInstance *i ) {

	target->set( new slBox( &BRVECTOR( &args[0] ), BRDOUBLE( &args[1] ) ) );

	return EC_OK;
}

int brINGonDiscNew( brEval args[], brEval *target, brInstance *i ) {

	target->set( slNewNGonDisc( BRINT( &args[0] ), BRDOUBLE( &args[1] ), BRDOUBLE( &args[2] ), BRDOUBLE( &args[3] ) ) );

	return EC_OK;
}

int brINGonConeNew( brEval args[], brEval *target, brInstance *i ) {

	target->set( slNewNGonCone( BRINT( &args[0] ), BRDOUBLE( &args[1] ), BRDOUBLE( &args[2] ), BRDOUBLE( &args[3] ) ) );

	return EC_OK;
}

int brIFreeShape( brEval args[], brEval *target, brInstance *i ) {
	slShape *s = BRSHAPEPOINTER( &args[0] );
	
	s -> release();

	return EC_OK;
}

int brIDataForShape( brEval args[], brEval *target, brInstance *i ) {
	void *serialShape;
	int length;
	slShape *s = BRSHAPEPOINTER( &args[0] );

	serialShape = s -> serialize( &length );
	target -> set( new brData( serialShape, length ) );

	slFree( serialShape );

	return EC_OK;
}

int brIShapeForData( brEval args[], brEval *target, brInstance *i ) {
	brData *d = BRDATA( &args[0] );

	if ( !d ) {
		target->set( (void*)NULL );
		return EC_OK;
	}

	target->set( slShape::deserialize(( slSerializedShapeHeader* )d->data ) );

	return EC_OK;
}

int brIShapeSetScale( brEval args[], brEval *target, brInstance *i ) {
	slShape *s = BRSHAPEPOINTER( &args[0] );
	s -> setScale( &BRVECTOR( &args[ 1 ] ) );

	return EC_OK;
}

int brIShapeGetScale( brEval args[], brEval *target, brInstance *i ) {
	slShape *s = BRSHAPEPOINTER( &args[0] );
	target -> set( *s -> scale() );

	return EC_OK;
}

int brIGetMass( brEval args[], brEval *target, brInstance *i ) {
	slShape *s = BRSHAPEPOINTER( &args[0] );

	target->set( s->getMass() );

	return EC_OK;
}

int brIGetDensity( brEval args[], brEval *target, brInstance *i ) {
	slShape *s = BRSHAPEPOINTER( &args[0] );

	target->set( s->getDensity() );

	return EC_OK;
}


int brIPointOnShape( brEval args[], brEval *target, brInstance *i ) {
	slShape *shape = BRSHAPEPOINTER( &args[0] );
	slVector *location = &BRVECTOR( &args[1] );
	slVector v;

	shape->pointOnShape( location, &v );

	target->set( v );

	return EC_OK;
}

/*@}*/

/**
 * Initializes internal breve functions related to shapes.
 */

void breveInitShapeFunctions( brNamespace *n ) {
	brNewBreveCall( n, "newShape", brIShapeNew, AT_POINTER, 0 );
	brNewBreveCall( n, "addShapeFace", brIAddShapeFace, AT_NULL, AT_POINTER, AT_LIST, 0 );
	brNewBreveCall( n, "finishShape", brIFinishShape, AT_NULL, AT_POINTER, AT_DOUBLE, 0 );
	brNewBreveCall( n, "shapeSetDensity", brIShapeSetDensity, AT_NULL, AT_POINTER, AT_DOUBLE, 0 );
	brNewBreveCall( n, "shapeSetMass", brIShapeSetMass, AT_NULL, AT_POINTER, AT_DOUBLE, 0 );
	brNewBreveCall( n, "shapeSetInertiaMatrix", brIShapeSetInertiaMatrix, AT_NULL, AT_POINTER, AT_MATRIX, 0 );
	brNewBreveCall( n, "shapeSetScale", brIShapeSetScale, AT_NULL, AT_POINTER, AT_VECTOR, 0 );
	brNewBreveCall( n, "shapeGetScale", brIShapeGetScale, AT_VECTOR, AT_POINTER, 0 );
	brNewBreveCall( n, "newSphere", brISphereNew, AT_POINTER, AT_DOUBLE, AT_DOUBLE, 0 );
	brNewBreveCall( n, "meshShapeNew", brIMeshShapeNew, AT_POINTER, AT_STRING, AT_STRING, AT_DOUBLE, 0 );
	brNewBreveCall( n, "newCube", brICubeNew, AT_POINTER, AT_VECTOR, AT_DOUBLE, 0 );
	brNewBreveCall( n, "newNGonDisc", brINGonDiscNew, AT_POINTER, AT_INT, AT_DOUBLE, AT_DOUBLE, AT_DOUBLE, 0 );
	brNewBreveCall( n, "newNGonCone", brINGonConeNew, AT_POINTER, AT_INT, AT_DOUBLE, AT_DOUBLE, AT_DOUBLE, 0 );
	brNewBreveCall( n, "freeShape", brIFreeShape, AT_NULL, AT_POINTER, 0 );
	brNewBreveCall( n, "pointOnShape", brIPointOnShape, AT_VECTOR, AT_POINTER, AT_VECTOR, 0 );
	brNewBreveCall( n, "dataForShape", brIDataForShape, AT_DATA, AT_POINTER, 0 );
	brNewBreveCall( n, "shapeForData", brIShapeForData, AT_POINTER, AT_DATA, 0 );
	brNewBreveCall( n, "getMass", brIGetMass, AT_DOUBLE, AT_POINTER, 0 );
	brNewBreveCall( n, "getDensity", brIGetDensity, AT_DOUBLE, AT_POINTER, 0 );
}
