import { Request, Response, Router } from 'express';
import { useMockData } from './mock-data';

const router = Router();

const paths = {
    query: '/query'
};

/**
 * 查询滤镜列表
 */
router.get(paths.query, async(req: Request, res:Response) => {
    const { filter_list } = useMockData();
    console.error('Filter List:', filter_list);
    res.json(filter_list);
});



// **** Export default **** //

export default router;
