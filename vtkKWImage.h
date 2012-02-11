/*=========================================================================

  Program:   KWImage - Kitware Image IO Library
  Module:    $RCSfile: vtkKWImage.h,v $

  Copyright (c) Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __vtkKWImage_h
#define __vtkKWImage_h

#include "itkImageBase.h"
#include "itkImage.h"
#include "itkImageIOBase.h"
#include "itkVTKImageImport.h"
#include "vtkObject.h"

#include "vtkImageData.h"
#include "vtkImageImport.h"
#include "vtkImageExport.h"

/** \class KWImage
 *
 * \brief This class represents an image object.
 *
 * This class associates an internal ITK image and a VTK importer in such a way
 * that internally it can make available both image formats to ITK and VTK
 * classes.
 *
 */
class vtkKWImage : public vtkObject
{
public:
  static vtkKWImage * New();

  vtkTypeRevisionMacro(vtkKWImage, vtkObject);

  //BTX
  typedef itk::ImageBase<3>                 ImageBaseType;
  typedef ImageBaseType::Pointer            ImagePointer;
  typedef ImageBaseType::ConstPointer       ImageConstPointer;
  typedef itk::ImageIOBase::IOComponentType ITKScalarPixelType;
  //ETX
  // Set the untyped ITK image
  void SetITKImageBase( ImageBaseType * );

  // Return the pixel type using ITK enums.
  ITKScalarPixelType GetITKScalarPixelType() const;

  // Return the pixel type using VTK enums.
  int GetVTKScalarPixelType();

  vtkImageData * GetVTKImage();

  // Return the ITK image base. This is independent of the pixel type
  const ImageBaseType * GetITKImageBase() const;

protected:
  vtkKWImage();
  ~vtkKWImage();
private:
  //BTX
  vtkKWImage(const vtkKWImage &);     // Not implemented.
  void operator=(const vtkKWImage &); // Not implemented.

  ImagePointer ItkImage;

  itk::ProcessObject::Pointer Exporter;

  vtkImageImport *Importer;
  //ETX
};

//BTX
template <class ImageType>
class VTKtoITKImage
{
public:
  typedef typename ImageType::Pointer             ImagePtr;
  typedef typename itk::VTKImageImport<ImageType> ImageImportType;
  typedef typename ImageImportType::Pointer       ImageImportPtr;
  VTKtoITKImage() : m_vtkImageData(0)
  {}

  void SetVTKImageData(vtkImageData *v) { this->m_vtkImageData = v; }
  ImagePtr GetITKImage();

private:
  vtkImageData *m_vtkImageData;
};

template <class ImageType>
typename ImageType::Pointer
VTKtoITKImage<ImageType>
::GetITKImage()
{
  vtkImageExport *vtkExport = vtkImageExport::New();
  ImageImportPtr importer = ImageImportType::New();

  vtkExport->SetInput(this->m_vtkImageData);
  importer->SetUpdateInformationCallback( vtkExport->GetUpdateInformationCallback() );
  importer->SetPipelineModifiedCallback( vtkExport->GetPipelineModifiedCallback() );
  importer->SetWholeExtentCallback( vtkExport->GetWholeExtentCallback() );
  importer->SetSpacingCallback( vtkExport->GetSpacingCallback() );
  importer->SetOriginCallback( vtkExport->GetOriginCallback() );
  importer->SetScalarTypeCallback( vtkExport->GetScalarTypeCallback() );
  importer->SetNumberOfComponentsCallback( vtkExport->GetNumberOfComponentsCallback() );
  importer->SetPropagateUpdateExtentCallback( vtkExport->GetPropagateUpdateExtentCallback() );
  importer->SetUpdateDataCallback( vtkExport->GetUpdateDataCallback() );
  importer->SetDataExtentCallback( vtkExport->GetDataExtentCallback() );
  importer->SetBufferPointerCallback( vtkExport->GetBufferPointerCallback() );
  importer->SetCallbackUserData( vtkExport->GetCallbackUserData() );
  importer->Update();
  ImagePtr rval = importer->GetOutput();
  vtkExport->Delete();
  return rval;
}

//ETX

#endif
