//
//
//

#ifndef DATASET_H
#define DATASET_H

namespace Datos
{
  class Dataset
  {
  public:
    virtual ~Dataset() = default;

    virtual int GetRecordCount() = 0;
  };

}


#endif //DATASET_H
