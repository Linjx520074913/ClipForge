import { Router } from 'express';

import filterRouter from './filter-router';


const apiRouter = Router();
apiRouter.use('/filter', filterRouter);

export default apiRouter;
